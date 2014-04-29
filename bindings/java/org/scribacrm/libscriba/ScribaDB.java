package org.scribacrm.libscriba;

import java.io.UnsupportedEncodingException;

public final class ScribaDB {

    static {
        System.loadLibrary("scriba-java");
    }

    // single parameter passed to database backend
    public static class DBParam {
        public String key;
        public String value;
    }

    // backend types
    public static class DBType {
        public static final byte BUILTIN = 0;
        public static final byte EXTERNAL = 1;
    }

    // database backend description
    public static class DBDescr {
        public String name;
        public byte type;
        // for external backends only
        public String location;
    }

    // library initialization and cleanup
    public static native int init(DBDescr descr, DBParam[] params);
    public static native void cleanup();

    // company routines
    public static native Company getCompany(long id);
    public static native DataDescriptor[] getAllCompanies();
    public static native DataDescriptor[] getCompaniesByName(String name);
    public static native DataDescriptor[] getCompaniesByJurName(String jur_name);
    public static native DataDescriptor[] getCompaniesByAddress(String address);
    public static native void addCompany(String name, String jur_name, String address,
                           String inn, String phonenum, String email);
    public static native void updateCompany(Company company);
    public static native void removeCompany(long id);

    // poc routines
    public static native POC getPoc(long id);
    public static native DataDescriptor[] getAllPeople();
    public static native DataDescriptor[] getPOCByName(String firstname,
                                                       String secondname,
                                                       String lastname);
    public static native DataDescriptor[] getPOCByCompany(long id);
    public static native DataDescriptor[] getPOCByPosition(String position);
    public static native DataDescriptor[] getPOCByEmail(String email);
    public static native void addPOC(String firstname, String secondname, String lastname,
                              String mobilenum, String phonenum, String email,
                              String position, long company_id);
    public static native void updatePOC(POC poc);
    public static native void removePOC(long id);

    // project routines
    public static native Project getProject(long id);
    public static native DataDescriptor[] getAllProjects();
    public static native DataDescriptor[] getProjectsByCompany(long company_id);
    public static native DataDescriptor[] getProjectsByState(byte state);
    public static native void addProject(String title, String descr, long company_id, byte state);
    public static native void updateProject(Project project);
    public static native void removeProject(long id);

    // event routines
    public static native Event getEvent(long id);
    public static native DataDescriptor[] getAllEvents();
    public static native DataDescriptor[] getEventsByCompany(long company_id);
    public static native DataDescriptor[] getEventsByPOC(long poc_id);
    public static native DataDescriptor[] getEventsByProject(long project_id);
    public static native void addEvent(String descr, long company_id, long poc_id, long project_id, byte type, String outcome, long timestamp, byte state);
    public static native void updateEvent(Event event);
    public static native void removeEvent(long id);

    public static byte[] getUtf8FromString(String str) throws UnsupportedEncodingException {
        if (str == null) {
            return null;
        }
        return str.getBytes("UTF-8");
    }

    public static String getStringFromUtf8(byte[] bytes) throws UnsupportedEncodingException {
        if (bytes == null) {
            return null;
        }
        return new String(bytes, "UTF-8");
    }
}
