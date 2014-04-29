package org.scribacrm.libscriba;

public final class Event {

    public static class Type {
        public static final byte MEETING = 0;
        public static final byte CALL = 1;
        public static final byte TASK = 2;
    }

    public final long id;
    public final String descr;
    public final long company_id;
    public final long poc_id;
    public final long project_id;
    public final byte type;
    public final String outcome;
    public final long timestamp;

    public Event(long id, String descr, long company_id, long poc_id, long project_id,
                 byte type, String outcome, long timestamp) {
        this.id = id;
        this.descr = descr;
        this.company_id = company_id;
        this.poc_id = poc_id;
        this.project_id = project_id;
        this.type = type;
        this.outcome = outcome;
        this.timestamp = timestamp;
    }
}
