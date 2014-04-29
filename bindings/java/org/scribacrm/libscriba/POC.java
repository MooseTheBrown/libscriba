package org.scribacrm.libscriba;

public final class POC {
   
    public final long id;
    public final String firstname;
    public final String secondname;
    public final String lastname;
    public final String mobilenum;
    public final String phonenum;
    public final String email;
    public final String position;
    public final long company_id; 

    public POC(long id, String firstname, String secondname, String lastname, String mobilenum,
               String phonenum, String email, String position, long company_id) {
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
}
