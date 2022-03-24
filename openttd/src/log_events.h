#pragma once

#include "stdafx.h"
#include <string>
#include "vehicle_base.h"
#include "station_base.h"
#include "strings_func.h"
#include "town_map.h"
#include "town.h"
#include "townname_func.h"
#include <hazelcast/client/hazelcast_client.h>
#include <cstddef>
#include <string>


// Stopping the train
#include "company_func.h"
#include "company_base.h"
#include <algorithm>
#include "vehicle_gui.h"
#include "command_func.h"
#include "command_type.h"
#include "debug.h"
#include "core/backup_type.hpp"


using namespace std;
using namespace hazelcast::client;
using namespace hazelcast::util;

string GetTheStationName(StationID s);
string GetRandomString(uint l, std::string charIndex);
bool LogVehicleEvent(Vehicle *v);
bool ShallTrainStop(Vehicle *v);


class JetResources {
public:

	static std::string JET_INPUT_MAP_NAME;

    static std::string JET_PREDICTION_MAP_NAME;


    std::shared_ptr<hazelcast_client> getJetClient() {
        return jetClient;
    }

    static std::shared_ptr<imap> getJetInputMap() {
        return jetClient->get_map(JET_INPUT_MAP_NAME).get();
    }

    static std::shared_ptr<imap> getJetPredictionMap() {
        return jetClient->get_map(JET_PREDICTION_MAP_NAME).get();
    }

    /**
     * Determines unique vehicle name from it's type and unit number
     * @return Vehicle name or empty string for error
     **/
    static std::string getVehicleName(Vehicle *v) {
        // determine vehicle ID
        UnitID unitnumber = v->unitnumber;///< unit number, for display purposes only

        if (unitnumber != 0) {
            string name;
            string type;
            
            switch (v->type) {
                case VEH_TRAIN:    type = "Train"; break;
                case VEH_ROAD:     type = "Road Vehicle"; break;
                case VEH_SHIP:     type = "Ship"; break;
                case VEH_AIRCRAFT: type = "Aircraft"; break;
                default:           type = "Unknown Vehicle";
            }

            if (v->name != NULL) {
                name = v->name;
            } else {
                name = type + " " + to_string(unitnumber);
            }

            return name;
        } else {
            return "";
        }
    }

private:

    static std::shared_ptr<hazelcast_client> newClient();

    static std::shared_ptr<hazelcast_client> jetClient;

};
