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

#define NUM_THREAD 4

static const int num_client = 16384;
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
        std::cout << "init_wsa() failed.\n";
        return -1;
    }

    heartbeat _heartbeat;
    
    _heartbeat.size = 0;
    _heartbeat.type = -1;

    core::udp::Endpoint server_endpoint(4000, "127.0.0.1");

    int opt = 1;
    for (int i = 0; i < num_client; i++) {
        core::udp::Endpoint endpoint(15150 + i, "127.0.0.1");
        sockets[i] = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        ::setsockopt(sockets[i], SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
        int ret = ::bind(sockets[i], (const sockaddr*)&endpoint.Addr(),
            sizeof(endpoint.Addr()));
        if (ret != 0) {
            std::cout << ret << std::endl;
            std::cout << WSAGetLastError() << std::endl;
        }
        ::connect(sockets[i], (const sockaddr*)&server_endpoint.Addr(),
            sizeof(server_endpoint.Addr()));
    }
    std::cerr << "join start\n";
    std::chrono::high_resolution_clock::time_point
        last(std::chrono::high_resolution_clock::now());
    std::thread threads[NUM_THREAD];
    
    for (int tid = 0; tid < NUM_THREAD; tid++) {
        threads[tid] = std::thread([tid]() {
            connect_packet packet;
            packet.size = 2;
            packet.type = -1;
            packet.seq = 0;
            for (int i = tid*(num_client / NUM_THREAD);
                i < (tid + 1)*(num_client / NUM_THREAD); i++) {
                packet.type = -1;
                packet.seq = i;
                ::send(sockets[i], (const char*)&packet, sizeof(packet), 0);
                ::recv(sockets[i], (char*)&packet, sizeof(packet), 0);
                packet.type = -3;
                ::send(sockets[i], (const char*)&packet, sizeof(packet), 0);
            }
        });
    }
    
    for (int tid = 0; tid < NUM_THREAD; tid++) threads[tid].join();

    std::chrono::high_resolution_clock::time_point
        curr(std::chrono::high_resolution_clock::now());
    double dt = std::chrono::duration<double, std::milli>(curr - last).count();
    std::cout << "join complete : " << dt << std::endl;

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