# Transport Tycoon Demo
[OpenTTD](http://openttd.org/) collision prevention using [Hazelcast](https://hazelcast.com/).

## About
OpenTTD (Open Transport Tycoon Deluxe) is an open source simulation game based upon the popular Microprose game "Transport Tycoon
Deluxe", written by Chris Sawyer. Players build the transport empire by transporting people and material between cities and
industries.

This demo extracts real-time vehicle data from OpenTTD and analyses it using the [Jet engine](https://docs.hazelcast.com/hazelcast/5.1/pipelines/overview#what-is-the-jet-engine)
data processing engine. The Jet application predicts train collisions. The predicted collision is pushed back to the running
OpenTTD game to stop the affected trains (and save lifes:-).

A goal is to demonstrate the possibilities of an in-memory streaming for a real-time (sub-second) processing on a large data streams.

Watch a [demo recording](https://www.youtube.com/watch?v=2RlmCZhhjMY)

[![Watch the demo](https://img.youtube.com/vi/2RlmCZhhjMY/0.jpg)](https://www.youtube.com/watch?v=2RlmCZhhjMY)

## Architecture

The source code of OpenTTD was changed to stream out the vehicle position data and receive back the predicted collisions. 
This approach was inspired by
[Using OpenTTD to create a realistic data stream](https://www.experts-exchange.com/articles/31095/Using-OpenTTD-to-create-a-realistic-data-stream.html)
blog. 

For each in-game vehicle, it's position update is sent to Hazelcast approximately 30x per second.  Position updates are serialized
as JSON messages. See an example:

```

{
	"Name":"Train 4",
	"TimeStamp":"1562933209898",
	"X position":1880,
	"Y position":1480
}


```

The position records are sent to Hazelcast cluster using Hazelcast messaging services ([EventJournal](https://docs.hazelcast.com/hazelcast/5.1/data-structures/event-journal)).

Hazelcast hosts a collision prediction job that is subscribed to the messaging service. It predicts the vehicle position by 
extendig the vehicle motion vector. In simple terms: the vehicle is expected to keep the direction and speed it carried from
now - 1 second to now. Collision is predicted if there is an intersection between two or more vectors. 

![Prediction visualised](/images/prediction.png)

This happens every 50 milliseconds. Higher resolution isn't possible due to the game speed (~30 game loops per second).

The collision prediction job is implemented using the [Pipeline API](https://docs.hazelcast.com/hazelcast/5.1/pipelines/overview)
of the Jet engine (see [the code](../../blob/master/vehicle-telemetry-analytics/src/main/java/biz/schr/impl/CollisionDetector.java#L37)).

Any predicted collision is stored to a [Key-Value store](https://docs.hazelcast.com/hazelcast/5.1/data-structures/map) in
Hazelcast. This also generates an event that gets delivered to the Hazelcast client running in the game. The event handler stops
affected vehicles using the OpenTTD API.

![Pipeline](/images/pipeline.png)

## Performance

The in-game code uses async Hazelcast APIs to avoid negative impact on game performance.

The server-side components benefit from the low-latency architecture of Hazelcast for both messaging and compute. As a result,
the whole prediction cycle takes just a few milliseconds to complete (mostly network-bound) giving enough headroom to stop the
running trains.

Server-side components are straightforward to scale to handle millions of events per second to support many game instances

## Structure

`openttd` directory contains a fork of OpenTTD 1.9.2 source code extended routines to export the data and handle the collision
events. See the `openttd/src/log_events.h` and `openttd/src/log_events.cpp` for the Hazelcast integration.

`vehicle-telemetry-analytics` contains the messaging and the analytical infrastructure.

`game-positions` contains the saved OpenTTD game positions that can be used for a demonstration


## Prerequisites  

* Java Development Kit 8+: [Installation Guide](https://docs.oracle.com/javase/8/docs/technotes/guides/install/install_overview.html)
* Apache Maven: [Installation Guide](https://maven.apache.org/install.html)
* Hazelcast C++ client: [Installation Guide](https://github.com/hazelcast/hazelcast-cpp-client/blob/v5.0.0/Reference_Manual.md#11-installing)

Prerequisites of OpenTTD are covered in next section.

## Building the Application

To build the Vehicle Telemetry Analytics, run:

```
cd vehicle-telemetry-analytics
mvn clean package
```

Install Hazelcast C++ client to your system.

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

Note for MacOS users: If starting the `openttd` binary fails, try building a Mac OS app bundle by running `make bundle` and start
the game by launching `OpenTTD` from the `bundle` folder and allow the app to access the Documents folder. 

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
* [Hazelcast C++ client 5.0](https://hazelcast.org/clients/cplusplus/)
* [Hazelcast 5.1](https://hazelcast.com/open-source-projects/downloads/)
