//
// main.cpp
//

#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <unordered_set>

#include "World.h"
#include "server.hpp"
#include "ThreadPool.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "udp_server.lib")
#pragma comment(lib, "thread_manager.lib")

#define IPS 4
#define INTERVAL 1000/IPS

#define CHAT_SERVER_IP "127.0.0.1"
#define CHAT_SERVER_PORT 55151

#define NUM_BTHREAD 1

std::mutex mtx_sessions[NUM_BTHREAD];
std::unordered_set<core::udp::Session*> sessions[NUM_BTHREAD];
std::mutex mtx_command;

World world;
core::udp::Packet p(5150, 0);
core::ThreadPool thread_pool(NUM_BTHREAD);

uint8_t command;
bool is_command_changed;

bool init_wsa()
{
    WSAData wsa_data;
    return WSAStartup(MAKEWORD(2, 2), &wsa_data) == 0;
}

void packet_handler(core::udp::Session*, core::udp::Packet&) { } // ignore packet

void accept_handler(core::udp::Session* session)
{
    static unsigned long long counter = -1;
    unsigned long long idx = InterlockedIncrement(&counter) % NUM_BTHREAD;
    mtx_sessions[idx].lock();
    std::cout << "Client accepted" << idx << " " << sessions[idx].size() << std::endl;
    sessions[idx].insert(session);
    session->Data() = new unsigned long long(idx);
    mtx_sessions[idx].unlock();
}

void disconnect_handler(core::udp::Session* session)
{
    std::cout << "Client disconnected.\n";
    if (session->Data() == nullptr) return;
    unsigned long long idx = *(unsigned long long*)session->Data();
    mtx_sessions[idx].lock();
    sessions[idx].erase(session);
    mtx_sessions[idx].unlock();
}

void broadcast(Snapshot& snapshot, uint8_t command)
{
    // bool + ushort * 2 == 5
    p.SetSize(snapshot.header_.total_size_ + 9);
    p.SetType(command);

    char* packet = const_cast<char*>(p.Data());

    memcpy(packet, (char*)&snapshot + 2, 2);
    memcpy(packet + 1, (char*)&snapshot + 3, 2);
    memcpy(packet + 3, (char*)&snapshot + 5, 2);
    memcpy(packet + 5, (char*)&snapshot + 7, 2);
    memcpy(packet + 7, (char*)&snapshot + 9, 2);
    memcpy(packet + 9, snapshot.data_->data(), snapshot.header_.total_size_);

    volatile unsigned long long counter = NUM_BTHREAD;
    for (int i = 0; i < NUM_BTHREAD; i++) {
        thread_pool.Enqueue([i, &counter]() mutable {
            mtx_sessions[i].lock();
            for (core::udp::Session* session : sessions[i]) {
                session->Send(p);
            }
            mtx_sessions[i].unlock();
            InterlockedDecrement(&counter);
        });
        /*
        std::thread([i,&counter]() mutable {
            mtx_sessions[i].lock();
            for (core::udp::Session* session : sessions[i]) {
                session->Send(p);
            }
            mtx_sessions[i].unlock();
            InterlockedDecrement(&counter);
       }).detach();
       */
    }
    while (counter);
}

void broadcast_task()
{
    unsigned int cnt = 0;
    while (true) {
        mtx_command.lock();
        uint8_t _command = is_command_changed ? command : 0;
        is_command_changed = false;
        mtx_command.unlock();
        char a = 0;

        world.SpawnEnemy();

        for (int i = 0; i < IPS; ++i) {
            world.ProcessCommand(static_cast<Command>(_command));
            world.MakeSnapshot();
            std::chrono::high_resolution_clock::time_point
                last(std::chrono::high_resolution_clock::now());
            broadcast(world.GetSnapshot(0), _command);
            std::chrono::high_resolution_clock::time_point
                curr(std::chrono::high_resolution_clock::now());
            double dt = std::chrono::duration<double, std::milli>(curr - last).count();
            dt = dt < INTERVAL ? dt : INTERVAL;
            Sleep(INTERVAL - dt);
            if (world.IsEnd()) {
                world.Init();
                std::cout << "World is End.\n";
                break;
            }
        }
    }
}

int main()
{
    if (!init_wsa()) {
        std::cout << "init_wsa() failed.\n";
        return -1;
    }

    core::udp::Server server(4000, 1);

    world.SetMapSize(13, 25);
    world.SetSnapshotStorageSize(16);
	world.SetIps(IPS);
    world.Init();

    server.SetAcceptHandler(&accept_handler);
    server.SetDisconnectHandler(&disconnect_handler);
    server.SetPacketHandler(&packet_handler);
    if (!server.RunNonBlock()) {
        std::cout << "Server run failed.\n";
        return -1;
    }

    // Communicate with chat server
    SOCKET socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in chat_server_addr;
    
    memset(&chat_server_addr, 0, sizeof(chat_server_addr));
    
    chat_server_addr.sin_family = AF_INET;
    chat_server_addr.sin_port = htons(CHAT_SERVER_PORT);
    InetPtonA(AF_INET, CHAT_SERVER_IP, &chat_server_addr.sin_addr);

    int ret = ::connect(socket,
        (sockaddr*)&chat_server_addr, sizeof(chat_server_addr));
    std::cout << WSAGetLastError() << std::endl;

    if (ret != 0) {
        WSACleanup();
        std::cout << "connection failed.\n";
        return 1;
    }
    
    std::cout << "connection successed.\n";

    std::thread broad_cast_thread([]() { broadcast_task(); });
    
    while (1)
    {
        uint8_t _command;

        recv(socket, reinterpret_cast<char*>(&_command),
            sizeof(_command), 0);
        mtx_command.lock();
        command = _command;
        is_command_changed = true;
        mtx_command.unlock();
        std::cout << "Received command : " << _command << std::endl;
        Sleep(1000);
    }

    return 0;
}