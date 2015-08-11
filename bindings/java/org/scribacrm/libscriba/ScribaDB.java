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

package org.scribacrm.libscriba;

import java.io.UnsupportedEncodingException;
import java.util.UUID;
import java.util.ArrayList;

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

    // merge strategies
    public static class MergeStrategy {
        public static final byte LOCAL_OVERRIDE = 0;
        public static final byte REMOTE_OVERRIDE = 1;
        public static final byte MANUAL = 2;
    }

    // merge status
    public static class MergeStatus {
        public static final byte OK = 0;
        public static final byte CONFLICTS = 1;
    }

    // library initialization and cleanup
    public static native int init(DBDescr descr, DBParam[] params);
    public static native void cleanup();

    // company routines
    public static native Company getCompany(UUID id);
    public static native DataDescriptor[] getAllCompanies();
    public static native DataDescriptor[] getCompaniesByName(String name);
    public static native DataDescriptor[] getCompaniesByJurName(String jur_name);
    public static native DataDescriptor[] getCompaniesByAddress(String address);
    public static native void addCompany(String name, String jur_name, String address,
                           String inn, String phonenum, String email);
    public static native void updateCompany(Company company);
    public static native void removeCompany(UUID id);

    // poc routines
    public static native POC getPoc(UUID id);
    public static native DataDescriptor[] getAllPeople();
    public static native DataDescriptor[] getPOCByName(String name);
    public static native DataDescriptor[] getPOCByCompany(UUID id);
    public static native DataDescriptor[] getPOCByPosition(String position);
    public static native DataDescriptor[] getPOCByEmail(String email);
    public static native void addPOC(String firstname, String secondname, String lastname,
                              String mobilenum, String phonenum, String email,
                              String position, UUID company_id);
    public static native void updatePOC(POC poc);
    public static native void removePOC(UUID id);

    // project routines
    public static native Project getProject(UUID id);
    public static native DataDescriptor[] getAllProjects();
    public static native DataDescriptor[] getProjectsByTitle(String title);
    public static native DataDescriptor[] getProjectsByCompany(UUID company_id);
    public static native DataDescriptor[] getProjectsByState(byte state);
    public static native void addProject(String title, String descr, UUID company_id, byte state,
                                         byte currency, long cost);
    public static native void updateProject(Project project);
    public static native void removeProject(UUID id);

    // event routines
    public static native Event getEvent(UUID id);
    public static native DataDescriptor[] getAllEvents();
    public static native DataDescriptor[] getEventsByDescr(String descr);
    public static native DataDescriptor[] getEventsByCompany(UUID company_id);
    public static native DataDescriptor[] getEventsByPOC(UUID poc_id);
    public static native DataDescriptor[] getEventsByProject(UUID project_id);
    public static native DataDescriptor[] getEventsByState(byte state);
    public static native void addEvent(String descr, UUID company_id, UUID poc_id, UUID project_id, byte type, String outcome, long timestamp, byte state);
    public static native void updateEvent(Event event);
    public static native void removeEvent(UUID id);

    // serialization routines
    public static native byte[] serialize(DataDescriptor[] companies,
                                          DataDescriptor[] events,
                                          DataDescriptor[] people,
                                          DataDescriptor[] projects);

    public static native byte deserialize(byte[] buf, byte mergeStrategy);

    // retrieve array of descriptors pointed to by nextId
    public static synchronized native DataDescriptor[] next(long nextId);

    // convenience wrapper around multiple next() calls:
    // load all parts of large data descriptor array using next() and
    // combine them into single array
    public static DataDescriptor[] fetchAll(DataDescriptor[] part) {
        ArrayList<DataDescriptor> concat = new ArrayList<DataDescriptor>();
        DataDescriptor[] cur_part = part;
        for (DataDescriptor d : cur_part) {
            if (d.nextId == DataDescriptor.NONEXT) {
                concat.add(d);
            }
        }
        while (cur_part[cur_part.length - 1].nextId != DataDescriptor.NONEXT) {
            cur_part = ScribaDB.next(cur_part[cur_part.length - 1].nextId);
            if (cur_part == null) {
                break;
            }
            for (DataDescriptor d : cur_part) {
                if (d.nextId == DataDescriptor.NONEXT) {
                    concat.add(d);
                }
            }
        }

        return concat.toArray(part);
    }

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
