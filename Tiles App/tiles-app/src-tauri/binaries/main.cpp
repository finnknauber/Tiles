#include "easywsclient.cpp"
#include <hidapi.h>
#include <json.hpp>
#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <wchar.h>
#include <chrono>
#include <thread>
#include <assert.h>
#include <string>
#include <format>
#include <fstream>


#ifdef _WIN32
#pragma comment( lib, "ws2_32" )
#include <WinSock2.h>
#endif


#define TypeGeneral 0b00010001
#define TypeParentManagement 0b00100010
#define TypeHereIsMyNID 0b00110011
#define TypeNeedNID 0b01000100
#define TypeReportHID 0b01010101
#define TypeReportNeighbours 0b01100110
#define TypeHereIsYourNID 0b01110111
#define TypeChangeHardwareID 0b10001000
#define TypeTileCommand 0b10011001

bool webserver = true;


unsigned char megabuff[64];
bool active_network_ids[255];
uint8_t hardware_ids[255][5];
uint8_t neighbours[255][4];
uint8_t active_neighbours[255];

bool update_config = false;

bool lastLED = false;
bool led = false;


void handle_message(const std::string & message)
{
    if (message == "toggle") {
        led = !led;
    }
    else if (message == "update-config") {
        update_config = true;
    }
}



bool HIDisZero(int one, int two, int three, int four) {
    if (one == 0 && two == 0 && three == 0 && four == 0) {
        return true;
    }
    return false;
}


int addHID(int nid, int one, int two, int three, int four, int five) {
    if (HIDisZero(one, two, three, four)) {
        return -1;
    }
    else if (hardware_ids[nid][0] != 0) {
        return -2;
    }
    else {
        hardware_ids[nid][0] = one;
        hardware_ids[nid][1] = two;
        hardware_ids[nid][2] = three;
        hardware_ids[nid][3] = four;
        hardware_ids[nid][4] = five;
        return 0;
    }
}


int getNewNetworkID() {
    for (int id = 2; id < 255; id++) {
        if (!active_network_ids[id]) {
            active_network_ids[id] = true;
            return id;
        }
    }
    return -1;
}


void removeNetworkID(int id) {
    active_network_ids[id] = false;
    hardware_ids[id][0] = 0;
    hardware_ids[id][1] = 0;
    hardware_ids[id][2] = 0;
    hardware_ids[id][3] = 0;
    hardware_ids[id][4] = 0;
    neighbours[id][0] = 0;
    neighbours[id][1] = 0;
    neighbours[id][2] = 0;
    neighbours[id][3] = 0;
}


void prepareBuffer(uint8_t type, uint8_t target, const int *data, uint8_t size) {
    for (int i = 0; i < 64; i++) {
		megabuff[i] = 0;
	}
	megabuff[0] = 0x0;
	megabuff[1] = type;
	megabuff[2] = target;
	megabuff[3] = 1;
	megabuff[4] = size;
	for (int i = 0; i < size; i++)
	{
		megabuff[5 + i] = data[i];
	}
}


void findNeighbours(uint8_t nid) {
    for (int i = 0; i < 4; i++) {
        if (neighbours[nid][i] != 0 and nid != neighbours[nid][i]) {
            if (!active_neighbours[neighbours[nid][i]]) {
                active_neighbours[neighbours[nid][i]] = true;
                findNeighbours(neighbours[nid][i]);
            }
        }
    }
}



using easywsclient::WebSocket;
static WebSocket::pointer ws = NULL;
using json = nlohmann::json;


int main() {
    using namespace std::this_thread;
    using namespace std::chrono;

    for (int i = 0; i < 255; i++) {
        active_network_ids[i] = false;
    }
    active_network_ids[1] = true;
    srand(time(0));

    std::ifstream f("C:\\Users\\finnk\\Desktop\\tiles-config.json");
    json data = json::parse(f);

    hid_device *device;
    int res;
    res = hid_init();

    if (webserver) {
        // *****WS SETUP***** //
        #ifdef _WIN32
            INT rc;
            WSADATA wsaData;

            rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
            if (rc) {
                printf("WSAStartup Failed.\n");
                return 1;
            }
        #endif
        ws = WebSocket::from_url("ws://localhost:8080");
        assert(ws);
        ws->send("Connected to Websocket!");
        // *****WS SETUP***** //
    }


    device = hid_open(0x2321, 0x8027, NULL);
    printf("Connecting to Core");
    if (webserver) {
        ws->send("Connecting to Core");
    }
    while (!device)
    {
        printf(".");
        sleep_for(milliseconds(500));
        device = hid_open(0x2321, 0x8027, NULL);
    }
    res = hid_set_nonblocking(device, 1);
    if (webserver) {
        ws->send("Connected");
        ws->send("{\"connected\": true}");
    }
    printf("\nConnected!\n");


    unsigned char buf[64];
    while (true) {
        if (led != lastLED) {
            ws->send("Toggle LED");
            int data[] = {1};
            if (led) {
                data[0] = {2};
            }
            prepareBuffer(TypeTileCommand, 2, data, 1);
            hid_write(device, megabuff, 64);
            lastLED = led;
        }

        if (webserver) {
            // *****WS STUFF***** //
            if (ws->getReadyState() != WebSocket::CLOSED) {
                ws->poll();
                ws->dispatch(handle_message);
            }
            // *****WS STUFF***** //
        }

        if (update_config) {
            printf("Updating config");
            ws->send("Updating config");
            std::ifstream f("C:\\Users\\finnk\\Desktop\\tiles-config.json");
            data = json::parse(f);
            update_config = false;
        }

        res = hid_read(device, buf, 64);
        if (res == -1) {
            const wchar_t* error = hid_error(device);
            if (webserver) {
                ws->send("{\"connected\": false}");
                ws->send("Error => Closing Connection!");
            }
            printf("Error: %ls => Closing Connection!\n", error);
            hid_close(device);
            neighbours[1][0] = 0;
            neighbours[1][1] = 0;
            neighbours[1][2] = 0;
            neighbours[1][3] = 0;
            for (int i = 2; i<255; i++) {
                removeNetworkID(i);
            }
            sleep_for(milliseconds(10));

            device = hid_open(0x2321, 0x8027, NULL);
            printf("Connecting to Core");
            if (webserver) {
                ws->send("Connecting to Core");
            }
            while (!device)
            {
                if (webserver) {
                    if (ws->getReadyState() != WebSocket::CLOSED) {
                        ws->poll();
                        ws->dispatch(handle_message);
                    }
                }
                printf(".");
                sleep_for(milliseconds(250));
                device = hid_open(0x2321, 0x8027, NULL);
            }
            res = hid_set_nonblocking(device, 1);
            printf("\nConnected!\n");
            if (webserver) {
                ws->send("Connected");
                ws->send("{\"connected\": true}");
            }
        }

        if (res != 0) {
            int type = buf[0];
            int target = buf[1];
            int sender = buf[2];
            int length = buf[3];


            if (type == TypeGeneral) {
                printf("Data Packet by %d: %d\n", sender, buf[4]);
                std::string arr = "Data Packet by " + std::to_string(sender) + ": " + std::to_string(buf[4]);
                if (webserver)
                    ws->send(arr);
                arr = "{\"sender\": " + std::to_string(sender) + ", \"data\": " + std::to_string(buf[4]) + "}";
                if (webserver)
                    ws->send(arr);
                if (hardware_ids[sender][0] == 0) {
                    removeNetworkID(sender);
                    std::string arr = "Removed Network ID (Data Packet by unknown sender): " + std::to_string(sender);
                    printf("Removed Network ID (Data Packet by unknown sender): %d\n", sender);
                    if (webserver)
                        ws->send(arr);
                    arr = "{\"removed_nid\": " + std::to_string(sender) + "}";
                    if (webserver)
                        ws->send(arr);
                    int data[] = {3};
                    prepareBuffer(TypeTileCommand, sender, data, 1);
                    hid_write(device, megabuff, 64);
                }
            }

            else if (type == TypeNeedNID) {
                int data[] = {getNewNetworkID()};
                printf("Network ID assigned: %d\n", data[0]);
                prepareBuffer(TypeHereIsYourNID, 0, data, 1);
                hid_write(device, megabuff, 64);
            }

            else if (type == TypeReportHID) {
                int result = addHID(sender, buf[4], buf[5], buf[6], buf[7], buf[8]);
                if (result == -1) {
                    int data[] = {rand() % 255 + 1, rand() % 255 + 1, rand() % 255 + 1, rand() % 255 + 1};
                    printf("Change Hardware ID %d: %d.%d.%d.%d\n", sender, data[0], data[1], data[2], data[3]);
                    prepareBuffer(TypeChangeHardwareID, sender, data, 4);
                    hid_write(device, megabuff, 64);
                }
                else if (result == -2) {
                    removeNetworkID(sender);
                    std::string arr = "Removed Network ID (Double Network ID): " + std::to_string(sender);
                    printf("Removed Network ID (Double Network ID): %d\n", sender);
                    if (webserver)
                        ws->send(arr);
                    arr = "{\"removed_nid\": " + std::to_string(sender) + "}";
                    if (webserver)
                        ws->send(arr);
                    int data[] = {3};
                    prepareBuffer(TypeTileCommand, sender, data, 1);
                    hid_write(device, megabuff, 64);
                }
                else {
                    std::string arr = "Reported Hardware ID -> " + std::to_string(sender) + ": " + std::to_string(buf[4]) + "." + std::to_string(buf[5]) + "." + std::to_string(buf[6]) + "." + std::to_string(buf[7]) + ":" + std::to_string(buf[8]);
                    printf("Reported Hardware ID -> %d: %d.%d.%d.%d:%d\n", sender, buf[4], buf[5], buf[6], buf[7], buf[8]);
                    if (webserver)
                        ws->send(arr);
                    arr = "{\"nid\": " + std::to_string(sender) + ", \"type\": " + std::to_string(buf[8]) + ", \"hid\": [" + std::to_string(buf[4]) + "," + std::to_string(buf[5]) + "," + std::to_string(buf[6]) + "," + std::to_string(buf[7]) + "]}";
                    if (webserver)
                        ws->send(arr);
                }
            }

            else if (type == TypeReportNeighbours) {
                if (sender == 0) {
                    printf("Not possible");
                }
                else if (hardware_ids[sender][0] == 0 and sender != 1 and sender != 0) {
                    removeNetworkID(sender);
                    std::string arr = "Removed Network ID (Report Neigbours by unknown sender): " + std::to_string(sender);
                    printf("Removed Network ID (Report Neigbours by unknown sender): %d\n", sender);
                    if (webserver)
                        ws->send(arr);
                    arr = "{\"removed_nid\": " + std::to_string(sender) + "}";
                    if (webserver)
                        ws->send(arr);
                    int data[] = {3};
                    prepareBuffer(TypeTileCommand, sender, data, 1);
                    hid_write(device, megabuff, 64);
                }
                else {
                    neighbours[sender][0] = buf[4];
                    neighbours[sender][1] = buf[5];
                    neighbours[sender][2] = buf[6];
                    neighbours[sender][3] = buf[7];
                    std::string arr = "Reported Neighbours -> " + std::to_string(sender) + ": " + std::to_string(buf[4]) + "." + std::to_string(buf[5]) + "." + std::to_string(buf[6]) + "." + std::to_string(buf[7]);
                    printf("Reported Neighbours -> %d: %d, %d, %d, %d\n", sender, buf[4], buf[5], buf[6], buf[7]);
                    if (webserver)
                        ws->send(arr);
                    arr = "{\"nid\": " + std::to_string(sender) + ", \"neighbours\": [" + std::to_string(buf[4]) + "," + std::to_string(buf[5]) + "," + std::to_string(buf[6]) + "," + std::to_string(buf[7]) + "]}";
                    if (webserver)
                        ws->send(arr);


                    for (int i = 0; i < 255; i++) {
                        active_neighbours[i] = false;
                    }
                    active_neighbours[1] = true;
                    findNeighbours(1);

                    for (int i = 2; i<255; i++) {
                        if (active_network_ids[i] and !active_neighbours[i]) {
                            removeNetworkID(i);
                            printf("Removed Network ID (not in neighbours): %d\n", i);
                            if (webserver) {
                                std::string arr = "Removed Network ID (not in neighbours): " + std::to_string(i);
                                ws->send(arr);
                                arr = "{\"removed_nid\": " + std::to_string(i) + "}";
                                ws->send(arr);
                            }
                        }
                    }
                }
            }
            else {
                printf("Unknown type by %d: %d\n", sender, type);
                std::string arr = "Unknown type by " + std::to_string(sender) + ": " + std::to_string(type);
                if (webserver)
                    ws->send(arr);
            }
        }
        sleep_for(milliseconds(10));
    }
    delete ws;
    #ifdef _WIN32
        WSACleanup();
    #endif
    hid_close(device);
    res = hid_exit();
    return 0;
}

