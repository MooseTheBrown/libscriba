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

package org.scribacrm.libscriba;

import java.util.UUID;

public final class DataDescriptor {

    public final UUID id;
    public final String descr;

    public DataDescriptor(UUID id, String descr) {
        this.id = id;
        this.descr = descr;
    }

    public DataDescriptor(long id_high, long id_low, String descr) {
        this.id = new UUID(id_high, id_low);
        this.descr = descr;
    }

    @Override
    public String toString() {
        return descr;
    }
}
