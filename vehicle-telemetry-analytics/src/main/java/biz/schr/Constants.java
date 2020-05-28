package biz.schr;

public class Constants {
    /**
     * Name of an input map. The Map Journal is used as an ingestion buffer.
     */
    public static final String INPUT_MAP_NAME = "openttd-events";
    /**
     * Name of an output map. Consumers register for change events to get predicted collisions.
     */
    public static final String PREDICTION_MAP_NAME = "openttd-predictions";

    public static final int PREDICTION_TTL_SECS = 2;

    // prediction window. We look back and we predict ahead using this this time frame
    public static final long PREDICT_POSITION_IN_MS = 1000;

    // check positions each 50 ms
    public static final long PREDICTION_INTERVAL_MS = 50;

    // when vehicles are in square of this size they are colliding
    public static final Double COLLISION_COORDINATE_RESOLUTION = 20.0;
}
