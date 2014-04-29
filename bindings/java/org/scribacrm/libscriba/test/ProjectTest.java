package org.scribacrm.libscriba.test;

import org.scribacrm.libscriba.*;
import org.junit.*;
import static org.junit.Assert.*;
import java.nio.file.*;
import java.io.IOException;

public class ProjectTest {

    private static final String testDBLocation = "./java_test_db";

    private long _company1_id = 0;
    private long _company2_id = 0;

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
        ScribaDB.addProject("Project 2", "Selling useless stuff",
                            _company1_id, Project.State.OFFER);
        ScribaDB.addProject("Project 3", "Selling other useless stuff",
                            _company2_id, Project.State.REJECTED);
    }

    @After
    public void tearDown() {
        ScribaDB.cleanup();
        try {
            Files.delete(FileSystems.getDefault().getPath(testDBLocation));
        } catch (IOException e) {}
    }

    @Test
    public void testGetProject() {
        DataDescriptor[] projects = ScribaDB.getAllProjects();
        assertEquals("should have 3 projects", 3, projects.length);

        Project project = ScribaDB.getProject(projects[0].id);
        assertNotNull("project 1 should exist", project);
        project = ScribaDB.getProject(projects[1].id);
        assertNotNull("project 2 should exist", project);
        project = ScribaDB.getProject(projects[2].id);
        assertNotNull("project 3 should exist", project);
    }

    @Test
    public void testProjectData() {
        DataDescriptor[] projects = ScribaDB.getProjectsByCompany(_company2_id);
        Project project = ScribaDB.getProject(projects[0].id);
        assertEquals("Project id match", projects[0].id, project.id);
        assertTrue("Project title match", project.title.equals("Project 3"));
        assertTrue("Project descr match", project.descr.equals("Selling other useless stuff"));
        assertEquals("Company id match", _company2_id, project.company_id);
        assertEquals("Project state match", Project.State.REJECTED, project.state);
    }

    @Test
    public void testSearchByCompany() {
        DataDescriptor[] projects = ScribaDB.getProjectsByCompany(_company1_id);
        assertEquals("should have 2 projects for company 1", 2, projects.length);

        projects = ScribaDB.getProjectsByCompany(_company2_id);
        assertEquals("should have 1 project for company 2", 1, projects.length);

        Project project = ScribaDB.getProject(projects[0].id);
        assertTrue("project 3 title match", project.title.equals("Project 3"));
    }

    @Test
    public void testSearchByState() {
        DataDescriptor[] projects = ScribaDB.getProjectsByState(Project.State.EXECUTION);
        assertEquals("1 project in execution state", 1, projects.length);
        projects = ScribaDB.getProjectsByState(Project.State.OFFER);
        assertEquals("1 project in offer state", 1, projects.length);
        projects = ScribaDB.getProjectsByState(Project.State.REJECTED);
        assertEquals("1 rejected project", 1, projects.length);
        projects = ScribaDB.getProjectsByState(Project.State.INITIAL);
        assertEquals("No projects in initial state", 0, projects.length);
    }

    @Test
    public void testUpdateProject() {
        DataDescriptor[] projects = ScribaDB.getProjectsByCompany(_company2_id);
        Project project = ScribaDB.getProject(projects[0].id);
        Project updated_project = new Project(project.id, project.title,
                                              project.descr, project.company_id,
                                              Project.State.PAYMENT);
        ScribaDB.updateProject(updated_project);
        Project check_project = ScribaDB.getProject(projects[0].id);
        assertEquals("project state should be updated", Project.State.PAYMENT,
                                                        check_project.state);
    }

    @Test
    public void testRemoveProject() {
        DataDescriptor[] projects = ScribaDB.getProjectsByCompany(_company2_id);
        ScribaDB.removeProject(projects[0].id);
        projects = ScribaDB.getProjectsByCompany(_company2_id);
        assertEquals("no projects should remain for company 2", 0, projects.length);
    }
}
