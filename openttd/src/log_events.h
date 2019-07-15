#pragma once

#include "stdafx.h"
#include <string>
#include "vehicle_base.h"
#include "station_base.h"
#include "strings_func.h"
#include "town_map.h"
#include "town.h"
#include "townname_func.h"
// #include <iostream>
// #include <fstream>
#include <hazelcast/client/HazelcastClient.h>
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

    class JetListener : public EntryListener<std::string, std::string> {
    public:
        virtual void entryAdded(const hazelcast::client::EntryEvent<std::string, std::string> &event) {
            std::cout << "Collision detected for vehicle " << event.getKey() << std::endl;

            // Iterate over all trains to find the affected one
            for (size_t i = 0; i < Vehicle::GetPoolSize(); i++) {
                Vehicle *v = Vehicle::Get(i);
                if (v == NULL) { 
                    continue;
                } else {

                    if ((!(v->vehstatus & VS_STOPPED) || v->cur_speed > 0)) {

                        std::string name = getVehicleName(v);
                        if (name == "") {
                            continue;
                        }
                    
                        if (event.getKey() == name) {
                            std::cout << "Stopping vehicle " << name << std::endl;                             
                        
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

        }

        virtual void entryRemoved(const hazelcast::client::EntryEvent<std::string, std::string> &event) {
        }

        virtual void entryUpdated(const hazelcast::client::EntryEvent<std::string, std::string> &event) {
        }

        virtual void entryEvicted(const hazelcast::client::EntryEvent<std::string, std::string> &event) {
        }

        virtual void entryExpired(const hazelcast::client::EntryEvent<std::string, std::string> &event) {
        }

        virtual void entryMerged(const hazelcast::client::EntryEvent<std::string, std::string> &event) {
        }

        virtual void mapEvicted(const hazelcast::client::MapEvent &event) {
        }

        virtual void mapCleared(const hazelcast::client::MapEvent &event) {
        }
    };

    static std::string JET_INPUT_MAP_NAME;

    static std::string JET_PREDICTION_MAP_NAME;

    

    boost::shared_ptr<HazelcastClient> getJetClient() {
        return jetClient;
    }

    static IMap<std::string, std::string> getJetInputMap() {
        return jetClient->getMap<std::string, std::string>(JET_INPUT_MAP_NAME);
    }

    static IMap<std::string, std::string> getJetPredictionMap() {
        return jetClient->getMap<std::string, std::string>(JET_PREDICTION_MAP_NAME);
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

    static boost::shared_ptr<HazelcastClient> newClient();

    static boost::shared_ptr<HazelcastClient> jetClient;
    
    static JetListener listener;


};