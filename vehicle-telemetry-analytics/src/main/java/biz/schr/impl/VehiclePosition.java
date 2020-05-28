package biz.schr.impl;

import com.hazelcast.internal.json.Json;
import com.hazelcast.internal.json.JsonObject;

import java.io.Serializable;
import java.util.Map;

public class VehiclePosition implements Serializable {

    public String name;

    public int xPos;

    public int yPos;

    public long timestamp;

    /**
     * Parses JSON into VehiclePosition object
     * @return
     */
    public static VehiclePosition parse(Map.Entry<String, String> event) {
        JsonObject vehicleJsonObject  = Json.parse(event.getValue()).asObject();

        VehiclePosition v = new VehiclePosition();
        v.name = event.getKey();
        v.xPos = vehicleJsonObject.get("X position").asInt();
        v.yPos = vehicleJsonObject.get("Y position").asInt();
        v.timestamp = Long.parseLong(vehicleJsonObject.get("TimeStamp").asString());

        return v;
    }


    @Override
    public String toString() {
        return "Vehicle " + name +
                " TS='" + timestamp + '\'' +
                " X='" + xPos + '\'' +
                " Y='" + yPos + '\'';
    }
}
