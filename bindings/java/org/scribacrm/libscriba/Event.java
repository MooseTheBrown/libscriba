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

    public final UUID id;
    public final String descr;
    public final UUID company_id;
    public final UUID poc_id;
    public final UUID project_id;
    public final byte type;
    public final String outcome;
    public final long timestamp;
    public final byte state;

    public Event(UUID id, String descr, UUID company_id, UUID poc_id, UUID project_id,
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

    public Event(long id_high, long id_low, String descr,
                 long company_id_high, long company_id_low,
                 long poc_id_high, long poc_id_low,
                 long project_id_high, long project_id_low,
                 byte type, String outcome, long timestamp, byte state) {
        this.id = new UUID(id_high, id_low);
        this.descr = descr;
        this.company_id = new UUID(company_id_high, company_id_low);
        this.poc_id = new UUID(poc_id_high, poc_id_low);
        this.project_id = new UUID(project_id_high, project_id_low);
        this.type = type;
        this.outcome = outcome;
        this.timestamp = timestamp;
        this.state = state;
    }
}
