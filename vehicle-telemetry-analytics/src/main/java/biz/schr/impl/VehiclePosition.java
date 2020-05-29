package biz.schr.impl;

import java.io.Serializable;

public class VehiclePosition implements Serializable {

    public String name;

    public int xPos;

    public int yPos;

    public long timestamp;

    @Override
    public String toString() {
        return "Vehicle " + name +
                " TS='" + timestamp + '\'' +
                " X='" + xPos + '\'' +
                " Y='" + yPos + '\'';
    }
}
