package biz.schr.impl;

import biz.schr.Constants;
import com.hazelcast.core.EntryEvent;
import com.hazelcast.core.HazelcastInstance;
import com.hazelcast.jet.JetInstance;
import com.hazelcast.jet.Traversers;
import com.hazelcast.jet.aggregate.AggregateOperations;
import com.hazelcast.jet.config.JobConfig;
import com.hazelcast.jet.datamodel.Tuple2;
import com.hazelcast.function.ComparatorEx;
import com.hazelcast.jet.pipeline.*;
import com.hazelcast.map.listener.EntryAddedListener;

import java.util.AbstractMap;

public class CollisionDetector {


    public static void start(HazelcastInstance hz) {
        // Print detected collisions to console for debugging
        hz.getMap(Constants.PREDICTION_MAP_NAME).addEntryListener(
                new CollisionAddedListener(), true);

        hz.getJet().newJob(buildPipeline(), new JobConfig().setName("Collision detector")).join();
    }

    private static Pipeline buildPipeline() {
        
        Pipeline p = Pipeline.create();

        p.readFrom(Sources.<String, String>mapJournal(Constants.INPUT_MAP_NAME, JournalInitialPosition.START_FROM_CURRENT))
                .withoutTimestamps().setName("Stream from buffer")
                .map(e -> VehiclePosition.parse(e)).setName("Parse JSON")
                .addTimestamps(v -> v.timestamp, 0)

                .groupingKey(vehiclePosition -> vehiclePosition.name)
                .window(WindowDefinition.sliding(Constants.PREDICT_POSITION_IN_MS, Constants.PREDICTION_INTERVAL_MS))
                .aggregate(AggregateOperations.allOf(
                        AggregateOperations.minBy(ComparatorEx.comparingLong(e -> e.timestamp)),
                        AggregateOperations.maxBy(ComparatorEx.comparingLong(e -> e.timestamp)),
                        (earliest, latest) -> Tuple2.tuple2(
                                latest.xPos + (latest.xPos - earliest.xPos),
                                latest.yPos + (latest.yPos - earliest.yPos)
                        )
                )).setName("Predict position")


                // for each vehicle we have predicted position in now + PREDICT_POSITION_IN_MS

                // group together items with the same predicted position
                // more coarse grained resolution (bigger "squares")
                .groupingKey( r -> Math.round( r.getValue().f0() / Constants.COLLISION_COORDINATE_RESOLUTION) +
                        "_" + Math.round(r.getValue().f1() / Constants.COLLISION_COORDINATE_RESOLUTION))

                // for each update interval, check vehicles that are in the same square
                .window(WindowDefinition.tumbling(Constants.PREDICTION_INTERVAL_MS))
                .aggregate(AggregateOperations.toList()).setName("Group co-located predictions")

                /// if there is more than 1 vehicle in the square predict the collision!
                .filter(l -> l.getValue().size() > 1).setName("Filter non-colliding")

                // put vehicles to the collision map. Short TTL - the prediction is relevant just before the collision happens
                .flatMap(l -> Traversers.traverseIterator(l.getValue().iterator() ) )
                .map( i -> new AbstractMap.SimpleEntry<String,String>(i.getKey(), i.getKey()))
                .writeTo(Sinks.map(Constants.PREDICTION_MAP_NAME)).setName("Save collisions to IMap");

        return p;
    }


    static class CollisionAddedListener implements EntryAddedListener<String,String> {

        @Override
        public void entryAdded(EntryEvent<String, String> event) {
            System.out.println("COLLISION " + event.getValue());
        }
    }

}
