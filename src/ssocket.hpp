#include <iostream>
#include <fstream>
#include <string.h>

#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <ws2tcpip.h>
#define GETSOCKETERRNO() (std::to_string(WSAGetLastError()))

#elif __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define GETSOCKETERRNO() (std::to_string(errno))
#endif


class SSocket {
    int af;
public:
    struct recvdata {
        char * value;
        size_t length;
    };
    struct address {
        std::string ip;
        int port;
    };
    struct sockaddr_in sock, client, my_addr;
#ifdef _WIN32
    WSADATA wsa;
    SOCKET s;
#elif __linux__
    int s;
#endif

    SSocket(int _af, int type) {
        af = _af;
#ifdef _WIN32
        WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
        if ((s = socket(af, type, 0)) == INVALID_SOCKET)
        {   
            throw GETSOCKETERRNO();
        }



    }
#ifdef _WIN32
    SSocket(SOCKET ss) {
        s = ss;
        WSAStartup(MAKEWORD(2, 2), &wsa);
    }
#elif __linux__
    SSocket(int ss) {
        s = ss;
    }
#endif


    void sconnect(std::string ip, int port) {
        if (ip != "") {
            if (ip.length() > 3) {
                if (!isdigit(ip[0]) && !isdigit(ip[1]) && !isdigit(ip[2])) {
                    ip = sgethostbyname(ip);
                }
                sock.sin_addr.s_addr = inet_addr(ip.c_str());
            }   
        }
        else {
            sock.sin_addr.s_addr = INADDR_ANY;
        }
        sock.sin_family = af;
        sock.sin_port = htons(port);
        if (connect(s, (struct sockaddr*)&sock, sizeof(sock)) == SOCKET_ERROR) {
            throw GETSOCKETERRNO();
        }
    }

    void sbind(std::string ip, int port) {
        if (ip != "") {
            if (!isdigit(ip[0]) && !isdigit(ip[1]) && !isdigit(ip[2])) {
                
                ip = sgethostbyname(ip);
            }
            sock.sin_addr.s_addr = inet_addr(ip.c_str());
        }
        else {
            sock.sin_addr.s_addr = INADDR_ANY;
        }
        sock.sin_family = af;
        sock.sin_port = htons(port);

        if (bind(s, (struct sockaddr*)&sock, sizeof(sock)) == SOCKET_ERROR) {
            throw GETSOCKETERRNO();
        }

    }

    std::string sgethostbyname(std::string name) {
        struct hostent* remoteHost;
        struct in_addr addr;
        remoteHost = gethostbyname(name.c_str());
        addr.s_addr = *(u_long*)remoteHost->h_addr_list[0];
        return std::string(inet_ntoa(addr));
    }


    address sgetsockname() {
        memset(&my_addr, 0, sizeof(my_addr));
        int addrlen = sizeof(my_addr);
#ifdef _WIN32
        if (getsockname(s, (struct sockaddr*)&my_addr, &addrlen) == SOCKET_ERROR) {
            throw GETSOCKETERRNO();
        }
        
        
#elif __linux__
        socklen_t len = sizeof(my_addr);
        if (getsockname(s, (struct sockaddr*)&my_addr, &len) == SOCKET_ERROR){
            throw GETSOCKETERRNO();
        }
#endif
        address addr;
        addr.ip = inet_ntoa(my_addr.sin_addr);
        addr.port = my_addr.sin_port;
        return addr;
    }

    void ssetsockopt(int level, int optname, int optval) {
#ifdef _WIN32
        setsockopt(s, level, optname, (char *)&optval, sizeof((char *)optval));
#elif __linux__
        setsockopt(s, level, optname, &optval, sizeof(int));
#endif
    }

    int sgetsockopt(int level, int optname) {
        int err = 0;
        socklen_t err_size = sizeof(err);
        return getsockopt(s, level, optname, &err, &err_size);
    }

    void slisten(int clients) {
        if (listen(s, clients) == SOCKET_ERROR) {
            throw GETSOCKETERRNO();
        }
    }

    SSocket saccept() {
        int c = sizeof(struct sockaddr_in);
#ifdef _WIN32
        auto new_socket = accept(s, (struct sockaddr*)&client, &c);
#elif __linux__
        auto new_socket = accept(s, (struct sockaddr*)&client, (socklen_t*)&c);
#endif
        if (new_socket == INVALID_SOCKET) {
            throw GETSOCKETERRNO();
        }

        return SSocket(new_socket);
    }

    size_t ssend(std::string data) {
        size_t size;
#ifdef _WIN32
        size = send(s, data.c_str(), data.length(), 0);
#elif __linux__
        size = send(s, data.c_str(), data.length(), MSG_NOSIGNAL);
#endif
        return size;
    }

    size_t ssend(char * data, size_t length) {
        size_t size;
#ifdef _WIN32
        size = send(s, data, length, 0);
#elif __linux__
        // std::cout << "strlen: " << strlen(data) << std::endl;
        size = send(s, data, length, MSG_NOSIGNAL);
#endif
        return size;
    }

    size_t ssend_file(std::ifstream& file) {
        char buffer[65536];
        size_t size = 0;
        while (file.tellg() != -1) {
            file.read(buffer, 65536);
#ifdef _WIN32
        size += send(s, buffer, file.gcount(), 0);
#elif __linux__
        size += send(s, buffer, file.gcount(), MSG_NOSIGNAL);
#endif
        memset(buffer, 0, sizeof(buffer));
        }
        return size;
    }

    std::string srecv(int length) {
#ifndef _DISABLE_RECV_LIMIT
        if (length > 32768){
            std::cout << "Error: srecv max value 32768" << std::endl;
            exit(EXIT_FAILURE);
        }
        char buffer[32768] = {0};
#else
        char buffer[65536] = {0};
#endif
        recv(s, buffer, length, 0);
        return std::string(buffer);
    }

    recvdata srecv_char(int length) {
#ifndef _DISABLE_RECV_LIMIT
        if (length > 32768) {
            std::cout << "Error: srecv_char max value 32768" << std::endl;
            exit(EXIT_FAILURE);
        }
        char buffer[32768] = {0};
#else
        char buffer[65536] = {0};
#endif
        recvdata data;
        data.length = recv(s, buffer, length, 0);
        data.value = buffer;
        if (data.length > 65536) {data.length = 0;}
        return data;

    }

    void sclose() {
#ifdef _WIN32
        closesocket(s);
        WSACleanup();
#elif __linux__
        close(s);
#endif
    }
};
