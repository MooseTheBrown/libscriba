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

import java.util.UUID;

public final class DataDescriptor {

    public static final long NONEXT = -1;

    public final UUID id;
    public final String descr;
    /* If nextId is not equal to NONEXT, then this descriptor points to
       the next section of a descriptor array. Call ScribaDB.next(nextId)
       to retrieve the next portion of the array. Array size limit is set
       in JNI code.
     */
    public final long nextId;

    public DataDescriptor(UUID id, String descr) {
        this.id = id;
        this.descr = descr;
        this.nextId = NONEXT;
    }

    public DataDescriptor(long id_high, long id_low, String descr) {
        this.id = new UUID(id_high, id_low);
        this.descr = descr;
        this.nextId = NONEXT;
    }

    // this constructor is used by JNI code
    public DataDescriptor(long nextId) {
        this.id = null;
        this.descr = null;
        this.nextId = nextId;
    }

    @Override
    public String toString() {
        return descr;
    }
}
