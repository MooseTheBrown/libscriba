/* 
 * Copyright (C) 2014 Mikhail Sapozhnikov
 *
 * This file is part of libscriba.
 *
 * libscriba is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libscriba is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libscriba. If not, see <http://www.gnu.org/licenses/>.
 *
 */

package org.scribacrm.libscriba.test;

import org.scribacrm.libscriba.*;
import org.junit.*;
import static org.junit.Assert.*;
import java.nio.file.*;
import java.io.IOException;
import java.util.UUID;

public class CompanyTest {

    private static final String testDBLocation = "./java_test_db";

    @Before
    public void setUp() {
        ScribaDB.DBDescr descr = new ScribaDB.DBDescr();
        descr.name = "scriba_sqlite";
        descr.type = ScribaDB.DBType.BUILTIN;

        ScribaDB.DBParam[] params = new ScribaDB.DBParam[1];
        params[0] = new ScribaDB.DBParam();
        params[0].key = "db_loc";
        params[0].value = testDBLocation;

        ScribaDB.init(descr, params);

        // add a couple of companies
        ScribaDB.addCompany("TestCompany#1", "TestCompany#1_jur_name", "Test address 1",
                            "1234567890", "555-44-33", "test1@test1.com");
        ScribaDB.addCompany("TestCompany#2", "TestCompany#2_jur_name", "Test address 2",
                            "0987654321", "222-11-00", "test2@test2.com");
    }

    @After
    public void tearDown() {
        ScribaDB.cleanup();
        try {
            Files.delete(FileSystems.getDefault().getPath(testDBLocation));
        } catch (IOException e) {}
    }

    @Test
    public void testGetCompany() {
        DataDescriptor[] companies = ScribaDB.getAllCompanies();
        assertEquals("Should have 2 companies", 2, companies.length);

        Company company1 = ScribaDB.getCompany(companies[0].id);
        Company company2 = ScribaDB.getCompany(companies[1].id);
        assertNotNull("company 1 should exist", company1);
        assertNotNull("company 2 should exist", company2);
    }

    @Test
    public void testGetCompaniesyByName() {
        DataDescriptor[] companies = ScribaDB.getCompaniesByName("TestCompany#1");
        assertEquals("1 company should be found by exact name", 1, companies.length);

        companies = ScribaDB.getCompaniesByName("test");
        assertEquals("2 companies should be found", 2, companies.length);
    }

    @Test
    public void testGetCompaniesByJurName() {
        DataDescriptor[] companies = ScribaDB.getCompaniesByJurName("TestCompany#2_jur_name");
        assertEquals("1 company should be found by exact jur name", 1, companies.length);

        companies = ScribaDB.getCompaniesByJurName("JUR");
        assertEquals("2 companies should be found", 2, companies.length);
    }

    @Test
    public void testGetCompaniesByAddress() {
        DataDescriptor[] companies = ScribaDB.getCompaniesByAddress("Test address 1");
        assertEquals("1 company should be found by address", 1, companies.length);
    }

    @Test
    public void testAddCompany() {
        ScribaDB.addCompany("New test company", "New jur name", "New address",
                            "1111111111", "111-11-11", "new@new.com");
        DataDescriptor[] companies = ScribaDB.getCompaniesByName("New test company");
        assertEquals("1 new company should be found", 1, companies.length);
    }

    @Test
    public void testUpdateCompany() {
        // retrieve company data and update it
        DataDescriptor[] companies = ScribaDB.getCompaniesByName("TestCompany#2");
        Company company = new Company(companies[0].id, "Updated_TestCompany#2",
                                      "NewJurName", "New address", "9999999999",
                                      "5555555", "updated@test.com");
        ScribaDB.updateCompany(company);

        // retrieve data for the company and verify that it's updated
        company = ScribaDB.getCompany(companies[0].id);
        assertNotNull("Updated company should exist", company);
        assertTrue("Name match", company.name.equals("Updated_TestCompany#2"));
        assertTrue("Jur name match", company.jur_name.equals("NewJurName"));
        assertTrue("Address match", company.address.equals("New address"));
        assertTrue("INN match", company.inn.equals("9999999999"));
        assertTrue("phonenum match", company.phonenum.equals("5555555"));
        assertTrue("email match", company.email.equals("updated@test.com"));
    }

    @Test
    public void testRemoveCompany() {
        // remove one of the companies and verify that only one remains
        DataDescriptor[] companies = ScribaDB.getAllCompanies();
        ScribaDB.removeCompany(companies[0].id);
        companies = ScribaDB.getAllCompanies();
        assertEquals("only 1 company should remain", 1, companies.length);
    }

    @Test
    public void testCompanyLists() {
        // add people, projects and events connected to a company
        DataDescriptor[] companies = ScribaDB.getCompaniesByName("TestCompany#2");
        UUID company_id = companies[0].id;

        ScribaDB.addPOC("Mikhail", "Alekseevich", "Sapozhnikov", "999888777",
                        "6543210", "msapozhnikov@test.com", "SW engineer", company_id);
        ScribaDB.addPOC("Kseniia", "Nikolayevna", "Sapozhnikova", "1111222333",
                        "6543210", "ksapozhnikova@test.com", "The boss", company_id);

        DataDescriptor[] poc1 = ScribaDB.getPOCByName("Mikhail");
        UUID poc1_id = poc1[0].id;
        DataDescriptor[] poc2 = ScribaDB.getPOCByName("Kseniia");
        UUID poc2_id = poc2[0].id;

        ScribaDB.addProject("SW development", "Just moosing around really", company_id,
                            Project.State.EXECUTION);

        DataDescriptor[] project = ScribaDB.getProjectsByCompany(company_id);
        UUID project_id = project[0].id;

        ScribaDB.addEvent("Status meeting", company_id, poc1_id, project_id, 
                          Event.Type.MEETING, "Discussed Byzantine empire", 0,
                          Event.State.SCHEDULED);
        DataDescriptor[] event = ScribaDB.getEventsByCompany(company_id);
        UUID event_id = event[0].id;

        // now get company data and check that the lists (people, projects, events)
        // are populated correctly
        Company company = ScribaDB.getCompany(company_id);
        assertEquals("2 people in the company", 2, company.poc_list.length);
        assertTrue("poc id match", (poc1_id.equals(company.poc_list[0].id)) ||
                                   (poc1_id.equals(company.poc_list[1].id)));
        assertEquals("1 project in the company", 1, company.proj_list.length);
        assertTrue("project id match", project_id.equals(company.proj_list[0].id));
        assertEquals("1 event in the company", 1, company.event_list.length);
        assertTrue("event id match", event_id.equals(company.event_list[0].id));
    }

    @Test
    public void testNullFields() {
        ScribaDB.addCompany("Null test company", null, "testaddr", null, "123456789", null);
        DataDescriptor[] companies = ScribaDB.getCompaniesByName("Null test company");
        assertTrue("descr match", companies[0].descr.equals("Null test company"));
        Company company = ScribaDB.getCompany(companies[0].id);
        assertTrue("Company name match", company.name.equals("Null test company"));
        assertTrue("null jur name", company.jur_name == null);
        assertTrue("null inn", company.inn.equals("0000000000"));

        ScribaDB.addCompany(null, null, "testaddr1", "1234567890", null, "test@test.com");
        companies = ScribaDB.getCompaniesByAddress("testaddr1");
        assertTrue("null descr", companies[0].descr == null);
        company = ScribaDB.getCompany(companies[0].id);
        assertTrue("null company name", company.name == null);
    }
}
