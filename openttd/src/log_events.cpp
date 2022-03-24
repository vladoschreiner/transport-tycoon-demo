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

hazelcast::client::entry_listener make_hz_listener() {
    return hazelcast::client::entry_listener()
        .on_added([](hazelcast::client::entry_event &&event) {

        	// std::cout << "Collision detected for vehicle " << event.getKey() << std::endl;

            // Iterate over all trains to find the affected one
            for (size_t i = 0; i < Vehicle::GetPoolSize(); i++) {
                Vehicle *v = Vehicle::Get(i);
                if (v == NULL) { 
                    continue;
                } else {

                    if ((!(v->vehstatus & VS_STOPPED) || v->cur_speed > 0)) {

                        std::string name = JetResources::getVehicleName(v);
                        if (name == "") {
                            continue;
                        }
                    
                        if (event.get_key().get<std::string>().value() == name) {
                            //std::cout << "Stopping vehicle " << name << std::endl;                             
                        
                            // backup _current_company global; _current_company contains index of the company
                            Backup<CompanyByte> cur_company(_current_company, FILE_LINE);

                            // get owner company of this vehicle 
                            Company *c1 = Company::Get(v->owner);

                            // switch to the owner company
                            cur_company.Change(c1->index);

                            // Issue "stop the train" command
                            bool dcp = DoCommandP(v->tile, v->index, 0, CMD_START_STOP_VEHICLE, NULL, NULL, false);

                            // restore original company
                            cur_company.Restore();
                        }
                        

                    }
                }
            }
        })
        .on_removed([](hazelcast::client::entry_event &&event) {
            //std::cout << "[removed] " << event << std::endl;
        })
        .on_updated([](hazelcast::client::entry_event &&event) {
            //std::cout << "[added] " << event << std::endl;
        })
        .on_evicted([](hazelcast::client::entry_event &&event) {
            //std::cout << "[updated] " << event << std::endl;
        })
        .on_expired([](hazelcast::client::entry_event &&event) {
            //std::cout << "[expired] " << event << std::endl;
        })
        .on_merged([](hazelcast::client::entry_event &&event) {
            //std::cout << "[merged] " << event << std::endl;
        })
        .on_map_evicted([](hazelcast::client::map_event &&event) {
            //std::cout << "[map_evicted] " << event << std::endl;
        })
        .on_map_cleared([](hazelcast::client::map_event &&event) {
            //std::cout << "[map_cleared] " << event << std::endl;
        });
}

/**
 * Initiates position to Hazelcast cluster using Hazelcast C++ client
 * @See https://github.com/hazelcast/hazelcast-cpp-client
 */
std::shared_ptr<hazelcast_client> JetResources::newClient() {

    client_config config;
    config.set_cluster_name("dev");
    
    auto client = make_shared<hazelcast_client>(hazelcast::new_client(std::move(config)).get());

	auto map = client->get_map(JET_PREDICTION_MAP_NAME).get();
    map->add_entry_listener(make_hz_listener(), true).get();

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
	auto jetMap = JetResources::getJetInputMap();
	jetMap->put<std::string, std::string>(name, message).get();

	return true;

}