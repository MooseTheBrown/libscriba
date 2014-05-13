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

public final class Project {

    public static class State {
        public static final byte INITIAL = 0;
        public static final byte CLIENT_INFORMED = 1;
        public static final byte CLIENT_RESPONSE = 2;
        public static final byte OFFER = 3;
        public static final byte REJECTED = 4;
        public static final byte CONTRACT_SIGNED = 5;
        public static final byte EXECUTION = 6;
        public static final byte PAYMENT = 7;
    }

    public final long id;
    public final String title;
    public final String descr;
    public final long company_id;
    public final byte state;

    public Project(long id, String title, String descr, long company_id, byte state) {
        this.id = id;
        this.title = title;
        this.descr = descr;
        this.company_id = company_id;
        this.state = state;
    }
}
