package org.scribacrm.libscriba;

public final class DataDescriptor {

    public final long id;
    public final String descr;

    public DataDescriptor(long id, String descr) {
        this.id = id;
        this.descr = descr;
    }

    @Override
    public String toString() {
        return descr;
    }
}
