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
#include <hazelcast/client/hazelcast_client.h>
#include "map_func.h"
#include <chrono>

using namespace hazelcast::client;
using namespace hazelcast::util;



/**
 * Initiates position to Hazelcast cluster using Hazelcast C++ client
 * @See https://github.com/hazelcast/hazelcast-cpp-client
 */
std::shared_ptr<hazelcast_client> JetResources::newClient() {

    //ClientConfig config;
    //config.getGroupConfig().setName("jet");
    //config.getGroupConfig().setPassword("jet-pass");

    //auto hz = hazelcast::new_client().get();

    //boost::shared_ptr<hazelcast_client> client( hazelcast::new_client().get());
//    IMap<std::string, std::string> map = client->getMap<std::string, std::string>(JET_PREDICTION_MAP_NAME);
//    map.addEntryListener(listener, true);



    client_config config;
    config.set_cluster_name("jet");
    config.set_credentials(std::make_shared<hazelcast::client::security::username_password_credentials>("", "jet-pass"));
    
    auto client = make_shared<hazelcast_client>(hazelcast::new_client(std::move(config)).get());


/*
    boost::shared_ptr<hazelcast_client> client(hazelcast::new_client(std::move(config)).get());

    auto map = client.get_map(JET_PREDICTION_MAP_NAME).get();
    hazelcast::client::entry_listener hazelcast::client::entry_listener()
    auto listener = hazelcast::client::entry_listener()
            .on_added([](hazelcast::client::entry_event&& event) {
                std::cout << "[added] " << event << std::endl;
            });
    map->add_entry_listener(std::move(listener), true).get(); */
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

std::shared_ptr<hazelcast_client> JetResources::jetClient = JetResources::newClient();
//JetResources::JetListener JetResources::listener;




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
	//imap<std::string, std::string> jetMap = JetResources::getJetInputMap();
	//jetMap->put(name, message).get();	

	auto jetMap = JetResources::getJetInputMap();
	jetMap->put<std::string, std::string>(name, message).get();

	return true;

}
