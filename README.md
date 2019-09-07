# Transport Tycoon Demo
[OpenTTD](http://openttd.org/) collision prevention using [Hazelcast Jet](https://jet.hazelcast.org).

## About
OpenTTD (Open Transport Tycoon Deluxe) is an open source simulation game based upon the popular Microprose game "Transport Tycoon Deluxe", written by Chris Sawyer. A player builds his empire by transporting people and material between cities and industries.

This demo extracts real-time vehicle data from OpenTTD and analyses it using [Hazelcast Jet](https://jet.hazelcast.org) data processing engine. The analytical job in Jet predicts train collisions. A pedicted collision information is pushed back to the running OpenTTD game to stop the affected trains (and save lifes:-).

A goal is to demonstrate the possibilities of an in-memory streaming for a real-time (sub-second) processing on a large data streams.

Watch a [demo recording](https://www.youtube.com/watch?v=2RlmCZhhjMY)

[![Watch the demo](https://img.youtube.com/vi/2RlmCZhhjMY/0.jpg)](https://www.youtube.com/watch?v=2RlmCZhhjMY)

## Architecture

The source code of OpenTTD was changed to export vehicle position data. This approach was inspired by [Using OpenTTD to create a realistic data stream](https://www.experts-exchange.com/articles/31095/Using-OpenTTD-to-create-a-realistic-data-stream.html) blog. For each vehicle, the changed game code exports a JSON record with the telemetry approximatelly 30x per second.

JSON record example:

```

{
	"Name":"Train 4",
	"TimeStamp":"1562933209898",
	"X position":1880,
	"Y position":1480
}


```

The position records are sent to a the Hazelcast Jet cluster to be analysed. In-memory data structures of Hazelcast Jet are used for reliable low-latency messaging. OpenTTD is implemented in C++ and Hazelcast comes with C++ client. This allows a straightforward data extraction.

The message buffer is consumed by the analytical job residing in the Hazelcast Jet cluster. The analytical job observes the position updates for each vehicle. It predicts the vehicle position in next second based on a vehicle position one second ago and a vehicle position now (it simply extends it's motion vector). Collision is predicted if there is an intersection between two or more vectors. 

![Prediction visualised](/images/prediction.png)

This happens each 50 milliseconds. Higher resolution isn't possible as the frequency of the vehicle data is just 30 Hz (vehicle position update arrives each 30-40 millisecond for each vehicle).

The analytical job is implemented using the [Pipeline API](https://docs.hazelcast.org/docs/jet/latest/manual/#pipeline-api) of Hazelcast Jet (see [the code](../../blob/master/vehicle-telemetry-analytics/src/main/java/biz/schr/impl/CollisionDetector.java#L37)).

If a collision is detected, it's recorded to a K-V store in Hazelcast Jet. This store allows an event-driven programming. Storing new collision triggers an event that is delivered to the C++ client in OpenTTD. OpenTTD can stop the affected vehicles.

![Pipeline](/images/pipeline.png)

## Performance

TODO: 

In-memory approach to meet the low-latency requirements (collision detection + train stop duration must happen within 1 second - prediction forecast interval). Possible thanks to combining in-mem messaging, processing, event-driven programming. 

Scaling to handle millions of events per second to support many game instances 

Fault-tolerance using in-memory replication.


## Structure

`openttd` directory contains a fork of OpenTTD 1.9.2 source code extended with Hazelcast 3.11 C++ client to export the data and handle the collision events. See the `openttd/src/log_events.h` and `openttd/src/log_events.cpp` for the Hazelcast integration.

`vehicle-telemetry-analytics` contains the messaging and the analytical infrastructure: input message buffer, stream processing cluster with the analytical job, K-V store for the detected collisions

`game-positions` contains the saved OpenTTD game positions that can be used for a demonstration


## Prerequisites  

* Java Development Kit 8+: [Installation Guide](https://docs.oracle.com/javase/8/docs/technotes/guides/install/install_overview.html)
* Apache Maven: [Installation Guide](https://maven.apache.org/install.html)

Prerequisites of OpenTTD are covered in next section.

## Building the Application

To build the Vehicle Telemetry Analytics, run:

```
cd vehicle-telemetry-analytics
mvn clean package
```

To build OpenTTD, please follow the [Compiling instructions](https://github.com/OpenTTD/OpenTTD/#70-compiling). For troubleshooting, please follow the [Compiling OpenTTD guide](https://wiki.openttd.org/Compiling).

## Running the Application

Start Jet (Collision Detection is disabled now):

```
cd vehicle-telemetry-analytics
mvn exec:java -Dexec.mainClass="biz.schr.StartJet"
```

Start OpenTTD:
```
cd openttd/bin
./openttd
```

Load the game position from `game-positions/demo1.sav`. Start all deployed trains by hitting the green flag from the train overview window and see the tragedy.

![Pipeline](/images/start-trains.png)

Start the collision detector within Jet:

```
cd vehicle-telemetry-analytics
mvn exec:java -Dexec.mainClass="biz.schr.StartCollisionDetector"
```

Reload game position, launch trains and let Jet save some lifes.





## Licensing

This demo is licensed under GPL 2 license

Hazelcast Jet and Hazelcast C++ client are licensed under Apache 2 license

OpenTTD is licensed under GPL 2. See [more](https://github.com/OpenTTD/OpenTTD/#100-licensing)

## Versions used

* [OpenTTD 1.9.2](https://github.com/OpenTTD/OpenTTD/tree/1.9.2)
* [Hazelcast C++ client 3.11](https://hazelcast.org/clients/cplusplus/)
* [Hazelcast Jet 3.1](https://jet.hazelcast.org/download/)
