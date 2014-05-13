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

public final class Event {

    public static class Type {
        public static final byte MEETING = 0;
        public static final byte CALL = 1;
        public static final byte TASK = 2;
    }

    public static class State {
        public static final byte SCHEDULED = 0;
        public static final byte COMPLETED = 1;
        public static final byte CANCELLED = 2;
    }

    public final long id;
    public final String descr;
    public final long company_id;
    public final long poc_id;
    public final long project_id;
    public final byte type;
    public final String outcome;
    public final long timestamp;
    public final byte state;

    public Event(long id, String descr, long company_id, long poc_id, long project_id,
                 byte type, String outcome, long timestamp, byte state) {
        this.id = id;
        this.descr = descr;
        this.company_id = company_id;
        this.poc_id = poc_id;
        this.project_id = project_id;
        this.type = type;
        this.outcome = outcome;
        this.timestamp = timestamp;
        this.state = state;
    }
}
