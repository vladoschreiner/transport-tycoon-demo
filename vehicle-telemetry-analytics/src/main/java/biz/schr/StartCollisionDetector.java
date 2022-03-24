package biz.schr;

import com.hazelcast.client.HazelcastClient;
import com.hazelcast.core.HazelcastInstance;
import com.hazelcast.jet.Jet;
import com.hazelcast.jet.JetInstance;
import biz.schr.impl.CollisionDetector;

/**
 * Starts the Collision Detection pipeline
 */
public class StartCollisionDetector {

    public static void main(String[] args) {

        HazelcastInstance hz  = HazelcastClient.newHazelcastClient();

        CollisionDetector.start(hz);

    }




}
