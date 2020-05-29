package biz.schr.impl;

import java.io.Serializable;

public class VehiclePosition implements Serializable {

    public VehiclePosition() {
    }

    private String name;

    private int xPos;

    private int yPos;

    private long timestamp;

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public int getxPos() {
        return xPos;
    }

    public void setxPos(int xPos) {
        this.xPos = xPos;
    }

    public int getyPos() {
        return yPos;
    }

    public void setyPos(int yPos) {
        this.yPos = yPos;
    }

    public long getTimestamp() {
        return timestamp;
    }

    public void setTimestamp(long timestamp) {
        this.timestamp = timestamp;
    }

    @Override
    public String toString() {
        return "Vehicle " + name +
                " TS='" + timestamp + '\'' +
                " X='" + xPos + '\'' +
                " Y='" + yPos + '\'';
    }
}
