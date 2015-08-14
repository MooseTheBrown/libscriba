/*
 * Copyright (C) 2015 Mikhail Sapozhnikov
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

public class SerializerTest {

    private static final String testDBLocation = "./java_test_db";

    private UUID _company_id = null;
    private UUID _event_id = null;
    private UUID _poc_id = null;
    private UUID _project_id = null;

    @Before
    public void setUp() {
        ScribaDB.DBDescr descr = new ScribaDB.DBDescr();
        descr.name = "scriba_sqlite";
        descr.type = ScribaDB.DBType.BUILTIN;

        ScribaDB.DBParam[] params = new ScribaDB.DBParam[2];
        params[0] = new ScribaDB.DBParam();
        params[0].key = "db_loc";
        params[0].value = testDBLocation;
        params[1] = new ScribaDB.DBParam();
        params[1].key = "db_sync";
        params[1].value = "off";

        ScribaDB.init(descr, params);

        // add test data
        ScribaDB.addCompany("TestCompany", "TestCompany_jur_name", "Test address",
                            "1234567890", "555-44-33", "testcompany@test.com");
        DataDescriptor[] companies = ScribaDB.getCompaniesByName("TestCompany");
        _company_id = companies[0].id;

        ScribaDB.addPOC("Firstname", "Secondname", "Lastname", "111", "222",
                        "email@test.com", "test position", _company_id);
        DataDescriptor[] people = ScribaDB.getPOCByCompany(_company_id);
        _poc_id = people[0].id;

        ScribaDB.addProject("Project 1", "Doing nothing",
                            _company_id, Project.State.EXECUTION,
                            Project.Currency.RUB, 1000, 100);
        DataDescriptor[] projects = ScribaDB.getProjectsByCompany(_company_id);
        _project_id = projects[0].id;

        ScribaDB.addEvent("Test event", _company_id, _poc_id, _project_id,
                          Event.Type.TASK, "outcome", 0, Event.State.COMPLETED);
        DataDescriptor[] events = ScribaDB.getEventsByCompany(_company_id);
        _event_id = events[0].id;
    }

    @After
    public void tearDown() {
        ScribaDB.cleanup();
        try {
            Files.delete(FileSystems.getDefault().getPath(testDBLocation));
        } catch (IOException e) {}
    }

    @Test
    public void testSerializer() {
        DataDescriptor[] companies = ScribaDB.getAllCompanies();
        DataDescriptor[] events = ScribaDB.getAllEvents();
        DataDescriptor[] people = ScribaDB.getAllPeople();
        DataDescriptor[] projects = ScribaDB.getAllProjects();

        byte[] data = ScribaDB.serialize(companies, events, people, projects);

        // remove all data from local DB
        cleanDB();

        // restore data from buffer
        byte status = ScribaDB.deserialize(data, ScribaDB.MergeStrategy.REMOTE_OVERRIDE);
        assertEquals("Merge status", ScribaDB.MergeStatus.OK, status);

        // verify restored data
        Company company = ScribaDB.getCompany(_company_id);
        assertNotNull("Company restored", company);
        assertTrue("Company name match", company.name.equals("TestCompany"));

        Event event = ScribaDB.getEvent(_event_id);
        assertNotNull("Event restored", event);
        assertEquals(event.type, Event.Type.TASK);

        POC poc = ScribaDB.getPoc(_poc_id);
        assertNotNull("POC restored", poc);
        assertTrue("First name match", poc.firstname.equals("Firstname"));

        Project project = ScribaDB.getProject(_project_id);
        assertNotNull("Project restored", project);
        assertEquals("Project state match", Project.State.EXECUTION, project.state);
    }

    private void cleanDB() {
        DataDescriptor[] companies = ScribaDB.getAllCompanies();
        DataDescriptor[] events = ScribaDB.getAllEvents();
        DataDescriptor[] people = ScribaDB.getAllPeople();
        DataDescriptor[] projects = ScribaDB.getAllProjects();

        for (DataDescriptor company : companies) {
            ScribaDB.removeCompany(company.id);
        }
        for (DataDescriptor poc : people) {
            ScribaDB.removePOC(poc.id);
        }
        for (DataDescriptor project : projects) {
            ScribaDB.removeProject(project.id);
        }
        for (DataDescriptor event : events) {
            ScribaDB.removeEvent(event.id);
        }
    }
}
