package biz.schr;

import com.hazelcast.config.Config;
import com.hazelcast.config.EventJournalConfig;
import com.hazelcast.config.MapConfig;
import com.hazelcast.core.Hazelcast;
import com.hazelcast.core.HazelcastInstance;
import com.hazelcast.jet.Jet;
import com.hazelcast.jet.JetInstance;
import com.hazelcast.jet.config.JetConfig;
import biz.schr.impl.IngestionMonitor;

public class StartJet {


    public static void main(String[] args) {
        Config jc = Config.loadDefault();

        jc.getJetConfig().setEnabled(true);

        jc.setProperty("hazelcast.partition.count", "1");

        jc.getMapConfig(Constants.INPUT_MAP_NAME)
          .setEventJournalConfig( new EventJournalConfig()
                .setEnabled(true)
                .setCapacity(100_000)
                .setTimeToLiveSeconds(0));

        jc.addMapConfig( new MapConfig()
                        .setName(Constants.PREDICTION_MAP_NAME)
                        .setTimeToLiveSeconds(Constants.PREDICTION_TTL_SECS));

        // Uncomment if you want to use Management Center for monitoring
        // Get Management Center from:
        // https://jet.hazelcast.org/download/#management-center
        // jc.getMetricsConfig().setEnabled(true).setJmxEnabled(true);
        // jc.getMetricsConfig().setMetricsForDataStructuresEnabled(true);

        HazelcastInstance jet = Hazelcast.newHazelcastInstance(jc);

        IngestionMonitor.start(jet);

    }




}
