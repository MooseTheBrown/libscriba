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

public final class Company {

    public final long id;
    public final String name;
    public final String jur_name;
    public final String address;
    public final String inn;
    public final String phonenum;
    public final String email;
    public final DataDescriptor[] poc_list;
    public final DataDescriptor[] proj_list;
    public final DataDescriptor[] event_list;

    // constructor to be used by Java code outside of the library
    public Company(long id, String name, String jur_name, String address,
                   String inn, String phonenum, String email) {
        this.id = id;
        this.name = name;
        this.jur_name = jur_name;
        this.address = address;
        this.inn = inn;
        this.phonenum = phonenum;
        this.email = email;
        // lists are only populated by native code when fetching company data
        // from database
        this.poc_list = null;
        this.proj_list = null;
        this.event_list = null;
    }

    // constructor to be used by library native code
    public Company(long id, String name, String jur_name, String address,
                   String inn, String phonenum, String email, DataDescriptor[] poc_list,
                   DataDescriptor[] proj_list, DataDescriptor[] event_list) {
        this.id = id;
        this.name = name;
        this.jur_name = jur_name;
        this.address = address;
        this.inn = inn;
        this.phonenum = phonenum;
        this.email = email;
        this.poc_list = poc_list;
        this.proj_list = proj_list;
        this.event_list = event_list;
    }
}
