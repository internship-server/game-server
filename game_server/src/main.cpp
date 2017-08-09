//
// main.cpp
//

#include <iostream>
#include <chrono>
#include <thread>
#include <vector>

#include "World.h"
#include "udp_server/src/server.hpp"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "udp_server.lib")

#define IPS 4
#define INTERVAL 1000/IPS

std::mutex mtx_sessions;
std::vector<core::udp::Session*> sessions;
World world;

bool init_wsa()
{
    WSAData wsa_data;
    return WSAStartup(MAKEWORD(2, 2), &wsa_data) == 0;
}

void packet_handler(core::udp::Session*, core::udp::Packet&) { } // ignore packet

// how about tls?
void accpet_handler(core::udp::Session* session)
{
    mtx_sessions.lock();
    std::cout << "Client accepted\n";
    sessions.push_back(session);
    mtx_sessions.unlock();
}

void broad_cast(Snapshot& snapshot)
{
    mtx_sessions.lock();
    // bool + ushort * 2 == 5
    core::udp::Packet p(snapshot.header_.total_size_ + 9, 1);
    char* packet = const_cast<char*>(p.Data());

    memcpy(packet, (char*)&snapshot + 2, 1);
    memcpy(packet + 1, (char*)&snapshot + 3, 2);
    memcpy(packet + 3, (char*)&snapshot + 5, 2);
    memcpy(packet + 5, (char*)&snapshot + 7, 2);
    memcpy(packet + 7, (char*)&snapshot + 9, 2);
    memcpy(packet + 9, snapshot.data_->data(), snapshot.header_.total_size_);

    for (core::udp::Session* session : sessions) {
        session->Send(p);
    }
    mtx_sessions.unlock();
}

void broad_cast_task()
{
    std::chrono::high_resolution_clock::time_point last;
    std::chrono::high_resolution_clock::time_point curr;

    last = std::chrono::high_resolution_clock::now();

    while (true) {
        last = std::chrono::high_resolution_clock::now();
        unsigned int command = rand() % 5;
        std::cout << command << std::endl;
        world.SpawnEnemy();
		for (int i = 0; i < IPS; ++i) {
			world.ProcessCommand(static_cast<Command>(command));
			world.MakeSnapshot();
			broad_cast(world.GetSnapshot(0));
            Sleep(INTERVAL);
		}
        curr = std::chrono::high_resolution_clock::now();
        int dt = ((std::chrono::duration<double, std::milli>)(curr - last)).count();
        Sleep(1500 - dt);
    } 
}

int main()
{
    if (!init_wsa()) {
        std::cout << "init_wsa() failed.\n";
        return 1;
    }

    core::udp::Server server(4000, 1);

    world.SetMapSize(8, 8);
    world.SetSnapshotStorageSize(16);
	world.SetIps(IPS);
    world.Init();

    server.SetAcceptHandler(&accpet_handler);
    server.SetPacketHandler(&packet_handler);

    server.RunNonBlock();

    std::thread broad_cast_thread([]() { broad_cast_task(); });

    // Communicate with chat server
    while (1);

    return 0;
}