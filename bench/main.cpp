//
// main.cpp
//
// don't want to be bothered with OOP 
//

#define _WINSOCKAPI_

#include <iostream>

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#include "server.hpp"
#include "endpoint.hpp"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "udp_server.lib")

static const int num_client = 2500;
SOCKET sockets[num_client];

struct connect_packet {
    short size;
    short type;
    short seq;
};

struct heartbeat {
    short size;
    short type;
};

bool init_wsa()
{
    WSAData wsa_data;
    return WSAStartup(MAKEWORD(2, 2), &wsa_data) == 0;
}

int main()
{
    if (!init_wsa()) {
        std::cout << "iniw_wsa() failed.\n";
        return -1;
    }

    connect_packet packet;
    heartbeat _heartbeat;
    
    _heartbeat.size = 0;
    _heartbeat.type = -1;

    core::udp::Endpoint server_endpoint(4000, "127.0.0.1");

    for (int i = 0; i < num_client; i++) {
        core::udp::Endpoint endpoint(55150 + i, "127.0.0.1");
        sockets[i] = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        ::bind(sockets[i], (const sockaddr*)&endpoint.Addr(),
            sizeof(endpoint.Addr()));
        ::connect(sockets[i], (const sockaddr*)&server_endpoint.Addr(),
            sizeof(server_endpoint.Addr()));
    }

    packet.size = 2;
    packet.type = -1;
    packet.seq = 0;

    for (int i = 0; i < num_client; i++) {
        packet.type = -1;
        ::send(sockets[i], (const char*)&packet, sizeof(packet), 0);
        ::recv(sockets[i], (char*)&packet, sizeof(packet), 0);
        packet.type = -3;
        ::send(sockets[i], (const char*)&packet, sizeof(packet), 0);
        std::cout << i << std::endl;
    }
    std::cout << "@@";
    std::thread([&]() {
        while (true) {
            std::chrono::high_resolution_clock::time_point
                last(std::chrono::high_resolution_clock::now());
            for (int i = 0; i < num_client; i++) {
                ::send(sockets[i], (const char*)&_heartbeat, sizeof(_heartbeat), 0);
            }
            std::chrono::high_resolution_clock::time_point
                curr(std::chrono::high_resolution_clock::now());
            int dt = std::chrono::duration<double, std::milli>(curr - last).count();
            if (dt > 10000) {
                std::cout << "too late for heartbeat..\n";
                dt = 10000;
                //break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10000 - dt));
        }
    }).detach();

    // check packet interval
    char buf[6000];
    while (true) {
        std::chrono::high_resolution_clock::time_point
            last(std::chrono::high_resolution_clock::now());
        ::recv(sockets[0], buf, sizeof(buf), 0);
        std::chrono::high_resolution_clock::time_point
            curr(std::chrono::high_resolution_clock::now());
        std::cout << "interval : " <<
            std::chrono::duration<double, std::milli>(curr - last).count();
        std::cout << std::endl;
    }

    return 0;
}