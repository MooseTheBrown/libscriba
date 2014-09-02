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
import java.util.Date;
import java.util.UUID;

public class EventTest {

    private static final String testDBLocation = "./java_test_db";

    private UUID _company1_id = null;
    private UUID _company2_id = null;
    private UUID _project1_id = null;
    private UUID _project2_id = null;
    private UUID _poc1_id = null;
    private UUID _poc2_id = null;
    private long _cur_ts = 0;

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

        DataDescriptor[] companies = ScribaDB.getCompaniesByName("TestCompany#1");
        _company1_id = companies[0].id;
        companies = ScribaDB.getCompaniesByName("TestCompany#2");
        _company2_id = companies[0].id;

        // add projects
        ScribaDB.addProject("Project 1", "Doing nothing",
                            _company1_id, Project.State.EXECUTION);
        ScribaDB.addProject("Project 2", "Salt",
                            _company2_id, Project.State.PAYMENT);
        DataDescriptor[] projects = ScribaDB.getProjectsByCompany(_company1_id);
        _project1_id = projects[0].id;
        projects = ScribaDB.getProjectsByCompany(_company2_id);
        _project2_id = projects[0].id;

        // add people
        ScribaDB.addPOC("Mikhail", "Alekseevich", "Sapozhnikov", "999888777",
                        "6543210", "msapozhnikov@test.com", "SW engineer", _company1_id);
        ScribaDB.addPOC("Kseniia", "Nikolayevna", "Sapozhnikova", "1111222333",
                        "6543210", "ksapozhnikova@test.com", "The boss", _company2_id);
        DataDescriptor[] people = ScribaDB.getPOCByName("Mikhail", "Alekseevich", "Sapozhnikov");
        _poc1_id = people[0].id;
        people = ScribaDB.getPOCByName("Kseniia", "Nikolayevna", "Sapozhnikova");
        _poc2_id = people[0].id;

        // add events
        Date cur_date = new Date();
        _cur_ts = cur_date.getTime() / 1000; // in seconds
        ScribaDB.addEvent("Blahblah", _company1_id, _poc1_id, _project1_id,
                          Event.Type.TASK, "Nothing useful", _cur_ts,
                          Event.State.SCHEDULED);
        ScribaDB.addEvent("Lunch", _company2_id, _poc2_id, _project2_id,
                          Event.Type.MEETING, "Everyone's velvet", _cur_ts + 500,
                          Event.State.COMPLETED);
    }

    @After
    public void tearDown() {
        ScribaDB.cleanup();
        try {
            Files.delete(FileSystems.getDefault().getPath(testDBLocation));
        } catch (IOException e) {}
    }

    @Test
    public void testGetAllEvents() {
        DataDescriptor[] events = ScribaDB.getAllEvents();
        assertEquals("Total number of events", 2, events.length);
    }

    @Test
    public void testGetEvent() {
        DataDescriptor[] events = ScribaDB.getEventsByCompany(_company1_id);
        Event event = ScribaDB.getEvent(events[0].id);
        assertTrue("Event id match", events[0].id.equals(event.id));
        assertTrue("Event descr match", event.descr.equals("Blahblah"));
        assertTrue("Company id match", _company1_id.equals(event.company_id));
        assertTrue("Poc id match", _poc1_id.equals(event.poc_id));
        assertTrue("Project id match", _project1_id.equals(event.project_id));
        assertEquals("Event type match", Event.Type.TASK, event.type);
        assertTrue("Event outcome match", event.outcome.equals("Nothing useful"));
        assertEquals("Event timestamp match", _cur_ts, event.timestamp);
        assertEquals("Event state match", Event.State.SCHEDULED, event.state);
    }

    @Test
    public void testSearchByCompany() {
        DataDescriptor[] events = ScribaDB.getEventsByCompany(_company1_id);
        assertEquals("1 event in company 1", 1, events.length);
        events = ScribaDB.getEventsByCompany(_company2_id);
        assertEquals("1 event in company 2", 1, events.length);
    }

    @Test
    public void testSearchByPOC() {
        DataDescriptor[] events = ScribaDB.getEventsByPOC(_poc1_id);
        assertEquals("1 event for poc 1", 1, events.length);
        events = ScribaDB.getEventsByPOC(_poc2_id);
        assertEquals("1 event for poc 2", 1, events.length);
    }

    @Test
    public void testSearchByProject() {
        DataDescriptor[] events = ScribaDB.getEventsByProject(_project1_id);
        assertEquals("1 event for project 1", 1, events.length);
        events = ScribaDB.getEventsByProject(_project2_id);
        assertEquals("1 event for project 2", 1, events.length);
    }

    @Test
    public void testUpdateEvent() {
        DataDescriptor[] events = ScribaDB.getEventsByCompany(_company1_id);
        Event event = ScribaDB.getEvent(events[0].id);
        Event updated_event = new Event(event.id, event.descr, event.company_id,
                                        event.poc_id, event.project_id, Event.Type.CALL,
                                        "Still nothing useful", event.timestamp,
                                        Event.State.CANCELLED);
        ScribaDB.updateEvent(updated_event);
        Event check_event = ScribaDB.getEvent(events[0].id);
        assertEquals("event type should be updated", Event.Type.CALL, check_event.type);
        assertTrue("outcome should be updated", check_event.outcome.equals("Still nothing useful"));
        assertEquals("event state should be updated",
                     Event.State.CANCELLED,
                     check_event.state);
    }

    @Test
    public void testRemoveEvent() {
        DataDescriptor[] events = ScribaDB.getEventsByCompany(_company2_id);
        ScribaDB.removeEvent(events[0].id);
        events = ScribaDB.getEventsByPOC(_poc2_id);
        assertEquals("no events should remain for poc 2", 0, events.length);
        events = ScribaDB.getEventsByProject(_project2_id);
        assertEquals("no events should remain for project 2", 0, events.length);
        events = ScribaDB.getEventsByCompany(_company2_id);
        assertEquals("no events should remain for company 2", 0, events.length);
    }
}
