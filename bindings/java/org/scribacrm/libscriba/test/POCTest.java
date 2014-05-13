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

public class POCTest {

    private static final String testDBLocation = "./java_test_db";

    private long _company_id;

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

        // add a company
        ScribaDB.addCompany("TestCompany", "TestCompany_jur_name", "Test address",
                            "1234567890", "555-44-33", "test@test.com");

        DataDescriptor[] companies = ScribaDB.getCompaniesByName("TestCompany");
        _company_id = companies[0].id;

        // add people
        ScribaDB.addPOC("Mikhail", "Alekseevich", "Sapozhnikov", "999888777",
                        "6543210", "msapozhnikov@test.com", "SW engineer", _company_id);
        ScribaDB.addPOC("Kseniia", "Nikolayevna", "Sapozhnikova", "1111222333",
                        "6543210", "ksapozhnikova@test.com", "The boss", _company_id);
    }

    @After
    public void tearDown() {
        ScribaDB.cleanup();
        try {
            Files.delete(FileSystems.getDefault().getPath(testDBLocation));
        } catch (IOException e) {}
    }

    @Test
    public void testGetAllPeople() {
        DataDescriptor[] people = ScribaDB.getAllPeople();
        assertEquals("Total number of people", 2, people.length);
    }

    @Test
    public void testGetPoc() {
        DataDescriptor[] people = ScribaDB.getPOCByName("Kseniia", "Nikolayevna", "Sapozhnikova");
        POC poc = ScribaDB.getPoc(people[0].id);
        assertEquals("POC id match", people[0].id, poc.id);
        assertTrue("POC firstname match", poc.firstname.equals("Kseniia"));
        assertTrue("POC secondname match", poc.secondname.equals("Nikolayevna"));
        assertTrue("POC lastname match", poc.lastname.equals("Sapozhnikova"));
        assertTrue("POC mobilenum match", poc.mobilenum.equals("1111222333"));
        assertTrue("POC phonenum match", poc.phonenum.equals("6543210"));
        assertTrue("POC email match", poc.email.equals("ksapozhnikova@test.com"));
        assertTrue("POC position match", poc.position.equals("The boss"));
        assertEquals("Company id match", _company_id, poc.company_id);
    }

    @Test
    public void testSearchByName() {
        DataDescriptor[] people = ScribaDB.getPOCByName("Mikhail", "Alekseevich", "Sapozhnikov");
        assertEquals("poc1 should exist", 1, people.length);
        
        people = ScribaDB.getPOCByName(null, null, "Sapozhnikova");
        assertEquals("poc2 should exist", 1, people.length);
    }

    @Test
    public void testSearchByCompany() {
        DataDescriptor[] people = ScribaDB.getPOCByCompany(_company_id);
        assertEquals("2 people in 1 company", 2, people.length);
    }

    @Test
    public void testSearchByPosition() {
        DataDescriptor[] people = ScribaDB.getPOCByPosition("The boss");
        assertEquals("1 boss", 1, people.length);
    }

    @Test
    public void testSearchByEmail() {
        DataDescriptor[] people = ScribaDB.getPOCByEmail("msapozhnikov@test.com");
        assertEquals("1 msapozhnikov", 1, people.length);
    }

    @Test
    public void testUpdatePOC() {
        DataDescriptor[] people = ScribaDB.getPOCByName("Mikhail", null, "Sapozhnikov");
        long poc_id = people[0].id;

        POC poc = ScribaDB.getPoc(poc_id);
        POC updated_poc = new POC(poc_id, poc.firstname, poc.secondname, poc.lastname,
                                  poc.mobilenum, "123456", poc.email,
                                  poc.position, poc.company_id);
        ScribaDB.updatePOC(updated_poc);
        POC check_poc = ScribaDB.getPoc(poc_id);
        assertTrue("phonenum should be updated", check_poc.phonenum.equals("123456"));
    }

    @Test
    public void testRemovePOC() {
        DataDescriptor[] people = ScribaDB.getPOCByName("Mikhail", null, "Sapozhnikov");
        long poc_id = people[0].id;

        ScribaDB.removePOC(poc_id);

        people = ScribaDB.getPOCByCompany(_company_id);
        assertEquals("1 poc should remain", 1, people.length);
    }
}
