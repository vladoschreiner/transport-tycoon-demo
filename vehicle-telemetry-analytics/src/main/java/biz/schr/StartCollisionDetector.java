package biz.schr;

import com.hazelcast.jet.Jet;
import com.hazelcast.jet.JetInstance;
import biz.schr.impl.CollisionDetector;

/**
 * Starts the Collision Detection pipeline
 */
public class StartCollisionDetector {

    public static void main(String[] args) {

        JetInstance jet  = Jet.newJetClient();

        CollisionDetector.start(jet);

    }




}
