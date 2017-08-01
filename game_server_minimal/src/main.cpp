//
// main.cpp
//

#include "header.hpp"

#include <iostream>

struct test_packet {
    int x;
    //int y;
};

int main()
{
    // init wsa
    WSADATA wsa_data;
    int ret = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (ret != 0) return 1;

    SOCKET client_socket = ::socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    std::cout << client_socket << std::endl;

    SOCKADDR_IN client_addr;

    memset(&client_addr, 0, sizeof(client_addr));

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(CLIENT_PORT);
    InetPtonA(AF_INET, "127.0.0.1", &client_addr.sin_addr);

    test_packet sample_packet;
    
    sample_packet.x = 1000;
    //sample_packet.y = 600;

    while (1) {
        std::cout << "Send...\n";
        int ret = sendto(client_socket,
            reinterpret_cast<const char*>(&sample_packet),
            sizeof(sample_packet), 0,
            reinterpret_cast<const sockaddr*>(&client_addr),
            sizeof(client_addr));
        
        if (ret == SOCKET_ERROR) break;
        std::cout << "Sent! " << ret << "\n";
        Sleep(1000);
    }

    closesocket(client_socket);
    WSACleanup();
    return 0;
}