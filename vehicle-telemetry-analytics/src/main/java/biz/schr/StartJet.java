package biz.schr;

import com.hazelcast.config.EventJournalConfig;
import com.hazelcast.config.MapConfig;
import com.hazelcast.jet.Jet;
import com.hazelcast.jet.JetInstance;
import com.hazelcast.jet.config.JetConfig;
import biz.schr.impl.IngestionMonitor;

public class StartJet {


    public static void main(String[] args) {
        JetConfig jc = JetConfig.loadDefault();

        jc.getHazelcastConfig().setProperty("hazelcast.partition.count", "1");
        jc.getHazelcastConfig().addMapConfig(new MapConfig()
                .setName(Constants.INPUT_MAP_NAME)
                .setEventJournalConfig(new EventJournalConfig()
                        .setEnabled(true)
                        .setCapacity(100_000)
                        .setTimeToLiveSeconds(0)));
        jc.getHazelcastConfig().addMapConfig(new MapConfig()
                .setName(Constants.PREDICTION_MAP_NAME)
                .setTimeToLiveSeconds(Constants.PREDICTION_TTL_SECS)
        );

        // Uncomment if you want to use Management Center for monitoring
        // Get Management Center from:
        // https://jet.hazelcast.org/download/#management-center
        // jc.getMetricsConfig().setEnabled(true).setJmxEnabled(true);
        // jc.getMetricsConfig().setMetricsForDataStructuresEnabled(true);

        JetInstance jet = Jet.newJetInstance(jc);

        IngestionMonitor.start(jet);

    }


}
