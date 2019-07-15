#include "stdafx.h"
#include <string>
#include "vehicle_base.h"
#include "station_base.h"
#include "strings_func.h"
#include "town_map.h"
#include "town.h"
#include "townname_func.h"
#include <iostream>
#include <fstream>
#include <cstddef>
#include "log_events.h"
#include <ctime>
#include <hazelcast/client/HazelcastClient.h>
#include "map_func.h"
#include <chrono>

using namespace hazelcast::client;
using namespace hazelcast::util;



/**
 * Initiates position to Hazelcast cluster using Hazelcast C++ client
 * @See https://github.com/hazelcast/hazelcast-cpp-client
 */
boost::shared_ptr<HazelcastClient> JetResources::newClient() {

    ClientConfig config;
    config.getGroupConfig().setName("jet");
    config.getGroupConfig().setPassword("jet-pass");

    boost::shared_ptr<HazelcastClient> client(new HazelcastClient(config));
    IMap<std::string, std::string> map = client->getMap<std::string, std::string>(JET_PREDICTION_MAP_NAME);
    map.addEntryListener(listener, true);

    return client;
}

/**
 * Name of the remote data structure used for input. 
 * OpenTTD streams vehicle position updates to it.
 */
std::string JetResources::JET_INPUT_MAP_NAME("openttd-events");

/**
 * Name of the remote data structure with detected collisions.
 * OpenTTD listens for new entries in this structure using JetResources::listener
 * @see https://github.com/hazelcast/hazelcast-cpp-client#752-distributed-data-structure-events
 */
std::string JetResources::JET_PREDICTION_MAP_NAME("openttd-predictions");

boost::shared_ptr<HazelcastClient> JetResources::jetClient = JetResources::newClient();
JetResources::JetListener JetResources::listener;




/**
 * Sends the current position of the vehicle to remote storage
 */ 
bool LogVehicleEvent(Vehicle *v) {
	
	int x_pos = v->x_pos;
	int y_pos = v->y_pos;
	unsigned __int64 now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	string name = JetResources::getVehicleName(v);
	if (name == "") {
		return false;
	}
	
	//Build JSON message
	string message = "{";
	message = message + "\"Name\":\"" + name + "\"" + ",";
	message = message + "\"TimeStamp\":\"" + to_string(now) + "\"" + ",";
	message = message + "\"X position\":" + to_string(x_pos) + ",";
	message = message + "\"Y position\":" + to_string(y_pos);
	message = message + "}";

	// Stream message to Hazelcast Jet server
	IMap<std::string, std::string> jetMap = JetResources::getJetInputMap();
	jetMap.put(name, message);	

	return true;

}