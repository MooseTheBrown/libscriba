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

    public static class Currency {
        public static final byte RUB = 0;
        public static final byte USD = 1;
        public static final byte EUR = 2;
    }

    public static class TimeComp {
        public static final byte IGNORE = 0;
        public static final byte BEFORE = 1;
        public static final byte AFTER = 2;
    }

    public final UUID id;
    public final String title;
    public final String descr;
    public final UUID company_id;
    public final byte state;
    public final byte currency;
    public final long cost;
    public final long start_time;
    public final long mod_time;

    public Project(UUID id, String title, String descr, UUID company_id, byte state,
                   byte currency, long cost, long start_time, long mod_time) {
        this.id = id;
        this.title = title;
        this.descr = descr;
        this.company_id = company_id;
        this.state = state;
        this.currency = currency;
        this.cost = cost;
        this.start_time = start_time;
        this.mod_time = mod_time;
    }

    public Project(long id_high, long id_low, String title, String descr,
                   long company_id_high, long company_id_low, byte state,
                   byte currency, long cost, long start_time, long mod_time) {
        this.id = new UUID(id_high, id_low);
        this.title = title;
        this.descr = descr;
        this.company_id = new UUID(company_id_high, company_id_low);
        this.state = state;
        this.currency = currency;
        this.cost = cost;
        this.start_time = start_time;
        this.mod_time = mod_time;
    }
}
