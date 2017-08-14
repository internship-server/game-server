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

#define CHAT_SERVER_IP "127.0.0.1"
#define CHAT_SERVER_PORT 55151

std::mutex mtx_sessions;
std::vector<core::udp::Session*> sessions;

std::mutex mtx_command;

World world;

uint8_t command;
bool is_command_changed;

bool init_wsa()
{
    WSAData wsa_data;
    return WSAStartup(MAKEWORD(2, 2), &wsa_data) == 0;
}

void packet_handler(core::udp::Session*, core::udp::Packet&) { } // ignore packet

// how about tls?
void accept_handler(core::udp::Session* session)
{
    mtx_sessions.lock();
    std::cout << "Client accepted\n";
    sessions.push_back(session);
    mtx_sessions.unlock();
}

void broad_cast(Snapshot& snapshot, uint8_t command)
{
    mtx_sessions.lock();
    // bool + ushort * 2 == 5
    core::udp::Packet p(snapshot.header_.total_size_ + 9, command);
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
    while (true) {
        mtx_command.lock();
        uint8_t _command = is_command_changed ? command : 0;
        is_command_changed = false;
        mtx_command.unlock();

        std::cout << (int)_command << std::endl;
        world.SpawnEnemy();

        for (int i = 0; i < IPS; ++i) {
            world.ProcessCommand(static_cast<Command>(_command));
            world.MakeSnapshot();
            broad_cast(world.GetSnapshot(0), command);
            Sleep(INTERVAL);
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
        return 1;
    }

    core::udp::Server server(4000, 1);

    world.SetMapSize(13, 25);
    world.SetSnapshotStorageSize(16);
	world.SetIps(IPS);
    world.Init();

    server.SetAcceptHandler(&accept_handler);
    server.SetPacketHandler(&packet_handler);

    server.RunNonBlock();

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

    std::thread broad_cast_thread([]() { broad_cast_task(); });

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