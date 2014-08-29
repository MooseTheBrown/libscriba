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

public final class POC {
   
    public final UUID id;
    public final String firstname;
    public final String secondname;
    public final String lastname;
    public final String mobilenum;
    public final String phonenum;
    public final String email;
    public final String position;
    public final UUID company_id; 

    public POC(UUID id, String firstname, String secondname, String lastname, String mobilenum,
               String phonenum, String email, String position, UUID company_id) {
        this.id = id;
        this.firstname = firstname;
        this.secondname = secondname;
        this.lastname = lastname;
        this.mobilenum = mobilenum;
        this.phonenum = phonenum;
        this.email = email;
        this.position = position;
        this.company_id = company_id;
    }

    public POC(long id_high, long id_low, String firstname,
               String secondname, String lastname, String mobilenum,
               String phonenum, String email, String position,
               long company_id_high, long company_id_low) {
        this.id = new UUID(id_high, id_low);
        this.firstname = firstname;
        this.secondname = secondname;
        this.lastname = lastname;
        this.mobilenum = mobilenum;
        this.phonenum = phonenum;
        this.email = email;
        this.position = position;
        this.company_id = new UUID(company_id_high, company_id_low);
    }
}
