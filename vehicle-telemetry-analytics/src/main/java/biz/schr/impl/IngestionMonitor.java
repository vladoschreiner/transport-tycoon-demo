package biz.schr.impl;

import biz.schr.Constants;
import com.hazelcast.core.HazelcastInstance;
import com.hazelcast.jet.JetInstance;
import com.hazelcast.jet.aggregate.AggregateOperations;
import com.hazelcast.jet.config.JobConfig;
import com.hazelcast.jet.pipeline.*;

/**
 * Basic monitoring of the ingestion stream volume
 */
public class IngestionMonitor {

    public static void start(HazelcastInstance hz) {
        hz.getJet().newJob(buildPipeline(), new JobConfig().setName("Ingestion monitor"));
    }


    private static Pipeline buildPipeline() {
        Pipeline p = Pipeline.create();
        p.readFrom(Sources.<String, String>mapJournal(Constants.INPUT_MAP_NAME,
                        JournalInitialPosition.START_FROM_CURRENT))
                .withoutTimestamps().setName("Stream from buffer")
                .map( e -> VehiclePosition.parse(e)).setName("Parse JSON")
                .addTimestamps(v -> v.timestamp, 0)
                .window(WindowDefinition.tumbling(1000))
                .aggregate(AggregateOperations.counting()).setName("Count datapoints")
                .writeTo(Sinks.logger(a -> "Datapoints last second: " + a.result())).setName("log");
        return p;
    }
}
