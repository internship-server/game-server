//
// main.cpp
//

#include <chrono>
#include <thread>
#include <vector>

#include "World.h"
#include "udp_server/src/server.hpp"

#define FRAME_INTERVAL 16

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

void broad_cast(const char*)
{
    mtx_sessions.lock();
    for (core::udp::Session* session : sessions) {
        // Send
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
        world.ProcessCommand(static_cast<Command>(command));
        world.MakeSnapshot();
        const char* snapshot = *world.GetSnapshot(0);
        broad_cast(snapshot);
        curr = std::chrono::high_resolution_clock::now();
        
        int dt = ((std::chrono::duration<double, std::milli>)(curr - last)).count();
        Sleep(FRAME_INTERVAL - dt > 0 ? FRAME_INTERVAL - dt : 0);
    }
}
int main()
{
    world.SetMapSize(64, 16);
    world.SetSnapshotStorageSize(16);
    world.Init();
    std::thread broad_cast_thread([]() { broad_cast_task(); });

    // Communicate with chat server
    while (1);

    return 0;
}