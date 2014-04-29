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
