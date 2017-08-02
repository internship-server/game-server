//
// main.cpp
//

#include "header.hpp"

#include <iostream>

struct test_packet {
    short size;
    short type;
    short dir;
    short x;
    short y;
};

int main()
{
    // init wsa
    WSADATA wsa_data;
    int ret = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    int opt = 1;

    if (ret != 0) return 1;

    SOCKET server_socket = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKET client_socket; // just consider 2 clients
    SOCKET client_socket2;
    SOCKADDR_IN server_addr, client_addr;
    
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
    std::cout << server_socket << std::endl;

    memset(&client_addr, 0, sizeof(client_addr));
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    ::bind(server_socket, (const sockaddr*)&server_addr, sizeof(server_addr));
    ::listen(server_socket, 5);

    int client_addr_len = sizeof(client_addr);
    client_socket = ::accept(server_socket, (PSOCKADDR)&client_addr, &client_addr_len);
    client_socket2 = ::accept(server_socket, (PSOCKADDR)&client_addr, &client_addr_len);

    std::cerr << WSAGetLastError() << std::endl;

    test_packet packet;
    packet.size = sizeof(packet);
    packet.type = 0;
    packet.x = 300;
    packet.y = 300;

    while (true) {
        int comm;
        std::cerr << "Input command : ";
        std::cin >> packet.dir;
        send(client_socket, (const char*)&packet, sizeof(packet), 0);
        send(client_socket2, (const char*)&packet, sizeof(packet), 0);
    }

    closesocket(client_socket);
    WSACleanup();

    return 0;
}