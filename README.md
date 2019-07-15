# Transport Tycoon Demo
Collision prevention for [OpenTTD](http://openttd.org/)

## About
OpenTTD (Open Transport Tycoon Deluxe) is an open source simulation game based upon the popular Microprose game "Transport Tycoon Deluxe", written by Chris Sawyer. Player builds his transport empire by transporting people and material between cities and industries.

This demo extracts real-time vehicle data from OpenTTD and analyses it using [Hazelcast Jet](https://jet.hazelcast.org) data processing engine. The analytical job in Jet predicts train collisions. A pedicted collision information is pushed back to the running OpenTTD game to stop the affected trains (and save lifes:-).

A goal is to demonstrate the possibilities of an in-memory streaming for a real-time (sub-second) processing on a large data streams.

[![Watch the demo](https://img.youtube.com/vi/2RlmCZhhjMY/0.jpg)](https://www.youtube.com/watch?v=2RlmCZhhjMY)

## Architecture

Source code of OpenTTD was changed to export vehicle data. This approach was inspired by [Using OpenTTD to create a realistic data stream](https://www.experts-exchange.com/articles/31095/Using-OpenTTD-to-create-a-realistic-data-stream.html) blog. For each vehicle, the changed game code exports a JSON record with the telemetry approximatelly 30x per second.

JSON record example:

```

{
	"Name":"Train 4",
	"TimeStamp":"1562933209898",
	"X position":1880,
	"Y position":1480
}


```

Telemetry data records are sent to a the Hazelcast Jet cluster to be analysed. In-memory data structures of Hazelcast Jet are used for reliable low-latency messaging. OpenTTD is implemented in C++ and Hazelcast comes with C++ client. This allows a straightforward data extraction.

The message buffer is consumed by the analytical job residing in the same cluster. The analytical job observes the position and timestamp updates for each vehicle. It predicts vehicle position in next second based on a vehicle position one second ago and a vehicle position now (it simply extends it's motion vector). Collision is predicted if there is an intersection between two or more vectors. 

![Prediction visualised](/images/prediction.png)

This happens each 50 milliseconds. Higher resolution isn't possible as the frequency of the vehicle data is just 30 Hz (position update arrives each 30-40 milliseconds).

The analytical job is implemented using the Pipeline API of Hazelcast Jet (see the code).

If a collision is detected, it's recorded to an K-V store in Jet. This store allows en event-driven programming. Storing new collision triggers an event that is delivered to the C++ client in OpenTTD. OpenTTD can stop the affected vehicles.

![Pipeline](/images/pipeline.png)

## Performance

TODO: 

In-memory approach to meet the low-latency requirements (collision detection + train stop duration must happen within 1 second - prediction forecast interval). Possible thanks to combining in-mem messaging, processing, event-driven programming. 

Scaling to handle millions of events per second to support many game instances 

Fault-tolerance using in-memory replication.


## Structure

`openttd` directory contains fork of OpenTTD 1.9.2 source code extended with Hazelcast client to export data and handle collision events

`vehicle-telemetry-analytics` contains the analytical infrastructure: messaging, stream processing cluster and the analytical job

`game-positions` contains the saved OpenTTD game positions that can be used for demonstration


## Prerequisites  

* Java Development Kit 8+: Installation Guide
* Apache Maven: Installation Guide

Prerequisites of OpenTTD are covered in next section.

## Building the Application

To build the Vehicle Telemetry Analytics jobs, run:

```
cd vehicle-telemetry-analytics
mvn clean package
```

Please follow [Compiling instructions](https://github.com/OpenTTD/OpenTTD/#70-compiling). For troubleshooting please follow the [Compiling OpenTTD guide](https://wiki.openttd.org/Compiling).

## Running the Application

Start the Vehicle Telemetry Analytics jobs:

```
cd vehicle-telemetry-analytics
mvn exec:java
```

Start OpenTTD:
```
cd openttd/bin
./openttd
```

Load the game position from `game-positions/demo1.sav`. Start all deployed by hitting the green flag from the train overview window and let Jet save some lifes.

![Pipeline](/images/start-trains.png)


## Licensing

This demo is licensed under Apache 2 license

Hazelcast Jet and Hazelcast C++ cliet are licensed under Apache 2

OpenTTD is licensed under GPL 2. See [more](https://github.com/OpenTTD/OpenTTD/#100-licensing)

## Versions used

OpenTTD 1.9.2
Hazelcast C++ client 3.11
Hazelcast Jet 3.1