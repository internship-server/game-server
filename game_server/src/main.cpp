//
// main.cpp
//
#include <chrono>
#include <thread>
#include <vector>

#include "World.h"
#include "udp_server/src/server.hpp"

using namespace std::chrono_literals;

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

void broad_cast()
{
    mtx_sessions.lock();
    for (core::udp::Session* session : sessions) {
        // Send
    }
    mtx_sessions.unlock();
}

void broad_cast_task()
{
    while (true) {
        unsigned int command = rand() % 5;
        world.SpawnEnemy();
        world.ProcessCommand(static_cast<Command>(command));
        world.MakeSnapshot();
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