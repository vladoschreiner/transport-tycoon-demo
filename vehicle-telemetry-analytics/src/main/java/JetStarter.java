import com.hazelcast.config.EventJournalConfig;
import com.hazelcast.config.MapConfig;
import com.hazelcast.core.EntryEvent;
import com.hazelcast.jet.Jet;
import com.hazelcast.jet.JetInstance;
import com.hazelcast.jet.aggregate.AggregateOperations;
import com.hazelcast.jet.config.JetConfig;
import com.hazelcast.jet.config.JobConfig;
import com.hazelcast.jet.pipeline.*;
import com.hazelcast.map.listener.EntryAddedListener;

public class JetStarter {

    public static final String INPUT_MAP_NAME = "openttd-events";
    public static final int PREDICTION_TTL_SECS = 2;
    public static final String PREDICTION_MAP_NAME = "openttd-predictions";


    public static void main(String[] args) {
        JetConfig jc = JetConfig.loadDefault();

        jc.getMetricsConfig().setMetricsForDataStructuresEnabled(true);

        jc.getHazelcastConfig().setProperty("hazelcast.partition.count", "1");

        jc.getHazelcastConfig().addEventJournalConfig( new EventJournalConfig()
                .setMapName(INPUT_MAP_NAME)
                .setEnabled(true)
                .setCapacity(100_000)
                .setTimeToLiveSeconds(0));

        jc.getHazelcastConfig().addMapConfig( new MapConfig()
                .setName(PREDICTION_MAP_NAME)
                .setTimeToLiveSeconds(PREDICTION_TTL_SECS)
        );

        // Uncomment if you want to use Management Center for monitoring
        // Get Management Center from:
        // https://jet.hazelcast.org/download/#management-center
        // jc.getMetricsConfig().setEnabled(true).setJmxEnabled(true);

        JetInstance jet  = Jet.newJetInstance(jc);

        IngestionMonitor.start(jet);

        CollisionDetector.start(jet);

    }




}
