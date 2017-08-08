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

#define FRAME_INTERVAL 16
#define ips 4

std::mutex mtx_sessions;
std::vector<core::udp::Session*> sessions;
World world;

void packet_handler(core::udp::Session*, core::udp::Packet&) { } // ignore packet

// how about tls?
void accpet_handler(core::udp::Session* session)
{
    mtx_sessions.lock();
    sessions.push_back(session);
    mtx_sessions.unlock();
}

void broad_cast(Snapshot& snapshot)
{
    mtx_sessions.lock();
    // bool + ushort * 2 == 5
    core::udp::Packet p(snapshot.header_.total_size_, 0);

    std::cout << "enemy_number_    : " <<
        snapshot.header_.enemy_number_ << std::endl;
    std::cout << "obstacle_number_ : " <<
        snapshot.header_.obstacle_number_ << std::endl;
    std::cout << "total_size_      : " <<
        snapshot.header_.total_size_ << std::endl;

    char* packet = const_cast<char*>(p.Data());

    memcpy(packet, (char*)&snapshot + 2, 1);
    memcpy(packet + 1, (char*)&snapshot + 3, 2);
    memcpy(packet + 3, (char*)&snapshot + 5, 2);
    memcpy(packet + 5, snapshot.data_->data(), snapshot.header_.total_size_);

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
        unsigned int command = rand() % 5;
        
        last = std::chrono::high_resolution_clock::now();
        world.SpawnEnemy();
		for (int i = 0; i < ips; ++i) {
			world.ProcessCommand(static_cast<Command>(command));
			world.MakeSnapshot();
			//broad_cast(world.GetSnapshot(0));
		}
        curr = std::chrono::high_resolution_clock::now();
        
        int dt = ((std::chrono::duration<double, std::milli>)(curr - last)).count();
        //Sleep(FRAME_INTERVAL - dt > 0 ? FRAME_INTERVAL - dt : 0);
		world.Print();
		Sleep(1000);
    }
}
int main()
{
    core::udp::Server server(4000, 1);

    world.SetMapSize(8, 8);
    world.SetSnapshotStorageSize(16);
	world.SetIps(ips);
    world.Init();

    server.SetAcceptHandler(&accpet_handler);
    server.SetPacketHandler(&packet_handler);

    server.RunNonBlock();

    std::thread broad_cast_thread([]() { broad_cast_task(); });

    // Communicate with chat server
    while (1);

    return 0;
}