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

public class DataArrayPartTest {

    private static final String testDBLocation = "./java_test_db";

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

        for (int i = 0; i < 130; i++) {
            ScribaDB.addCompany("TestCompany#" + 1, "test_jur_name", "test_addr",
                                "1234567890", "111", "test@test.com");
        }
    }

    @After
    public void tearDown() {
        ScribaDB.cleanup();
        try {
            Files.delete(FileSystems.getDefault().getPath(testDBLocation));
        } catch (IOException e) {}
    }

    @Test
    public void testArrayPart() {
        DataDescriptor[] companies = ScribaDB.getAllCompanies();
        // native code should give us 49 companies and 1 "next" descriptor
        // (update this test if MAX_ARRAY_SIZE is updated in JNI code)
        assertEquals("First part should have 50 companies", 50, companies.length);

        for (int i = 0; i < 49; i++)
        {
            assertTrue("DataDescriptor[" + i + "] should have invalid next pointer",
                       (companies[i].nextId == DataDescriptor.NONEXT));
        }

        assertTrue("Last descriptor should point to next part",
                   (companies[49].nextId != DataDescriptor.NONEXT));

        DataDescriptor[] next1 = ScribaDB.next(companies[49].nextId);
        assertEquals("Second part should have 50 companies", 50, next1.length);

        for (int i = 0; i < 49; i++)
        {
            assertTrue("DataDescriptor[" + i + "] should have invalid next pointer",
                       (next1[i].nextId == DataDescriptor.NONEXT));
        }

        assertTrue("Last descriptor should point to next part",
                   (next1[49].nextId != DataDescriptor.NONEXT));

        // there should remain 130 - 49 - 49 = 32 companies
        DataDescriptor[] next2 = ScribaDB.next(next1[49].nextId);
        assertEquals("Third part should have 32 companies", 32, next2.length);

        for (int i = 0; i < 32; i++)
        {
            assertTrue("DataDescriptor[" + i + "] should have invalid next pointer",
                       (next2[i].nextId == DataDescriptor.NONEXT));
        }
    }

    @Test
    public void testFetchAll() {
        DataDescriptor[] part = ScribaDB.getAllCompanies();
        DataDescriptor[] all = ScribaDB.fetchAll(part);
        assertEquals("All 130 companies are retrieved", 130, all.length);
    }
}
