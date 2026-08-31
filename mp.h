// multiplayer
#include "tiny_engine.h"
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #ifdef _MSC_VER
        #pragma comment(lib, "ws2_32.lib") 
    #endif
    typedef int socklen_t;
    #define CLOSE_SOCKET closesocket
    #define SEND_FLAGS 0
    #define RECV_FLAGS 0 
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #define CLOSE_SOCKET close
    #ifdef __linux__
        #define SEND_FLAGS MSG_CONFIRM
    #else
        #define SEND_FLAGS 0
    #endif
    #define RECV_FLAGS 0
#endif

#define PORT         8080 
#define MAX_LINE     1024 
#define NAME_LEN     20
#define CREATION_LEN 50

struct payload
{
    int index;
    char name[NAME_LEN];
    glm::mat4 mat;
};

struct creation_payload
{
    int index = -1;
    char name[NAME_LEN];
    char gltf[CREATION_LEN];
    char start = 0; 
};

class Mp
{
public:
    Mp() {
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed" << std::endl;
            exit(EXIT_FAILURE);
        }
        sockfd = INVALID_SOCKET;
#else
        sockfd = -1;
#endif
        players = nullptr;
    }

    void set_player(const char name[NAME_LEN], const char gltf[CREATION_LEN], std::string server_ip, Tiny_engine *engine, std::string scene_name)
    {
        strcpy(crp.name, name);
        strcpy(crp.gltf, gltf);

        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
#ifdef _WIN32
        if (sockfd == INVALID_SOCKET)
#else
        if (sockfd < 0)
#endif
        {
            perror("socket creation failed");
            exit(EXIT_FAILURE);
        }
        memset(&servaddr, 0, sizeof(servaddr));
        
#ifdef _WIN32
        DWORD timeout = 1000; 
        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) == SOCKET_ERROR) {
#else
        struct timeval timeout = {
            .tv_sec = 1,
            .tv_usec = 0
        };
        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) == -1) {
#endif
            perror("setsockopt(SO_RCVTIMEO)");
            CLOSE_SOCKET(sockfd);
            exit(EXIT_FAILURE);
        }

        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(PORT);
        servaddr.sin_addr.s_addr = inet_addr(server_ip.c_str());

        len = sizeof(servaddr);

        sendto(sockfd, (const char*)&crp, (int)sizeof(crp), SEND_FLAGS,
               (const struct sockaddr *)&servaddr, len);

        int n = recvfrom(sockfd, buffer, (int)MAX_LINE, RECV_FLAGS,
                         (struct sockaddr *)&servaddr, &len);
        if (n <= 0) {
            std::cerr << "Failed to receive response from server" << std::endl;
            CLOSE_SOCKET(sockfd);
            exit(EXIT_FAILURE);
        }

        crp.index = *(int*)buffer;
        this->engine = engine;
        this->scene_name = scene_name;
    }

    void start()
    {
        crp.start = 1;
        memset(buffer, 0, MAX_LINE);

        sendto(sockfd, (const char*)&crp, (int)sizeof(crp), SEND_FLAGS,
               (const struct sockaddr *)&servaddr, len);

        int n = recvfrom(sockfd, buffer, (int)MAX_LINE, RECV_FLAGS,
                         (struct sockaddr *)&servaddr, &len);
        if (n <= 0) return;

        creation_payload *all_crps = (creation_payload*)malloc(n);
        memcpy(all_crps, buffer, n);

        int num_players = n / (int)sizeof(creation_payload);
        for (int i = 0; i < num_players; ++i)
        {
            std::cout << i << std::endl;
            tiny_engine::Object obj;
            obj.obj_name = all_crps[i].name;
            obj.scene_name = this->scene_name;
            obj.pos = glm::mat4(1);
            obj.gltf_model_path = all_crps[i].gltf;

            engine->addObject(obj);
        }
        players = (payload*)malloc(num_players * sizeof(payload));
        free(all_crps);
    }

    void update(glm::mat4 mat)
    {
        payload p;
        p.mat = mat;
        p.index = crp.index;        
        memcpy(p.name, crp.name, NAME_LEN);
        
        sendto(sockfd, (const char*)&p, (int)sizeof(p), SEND_FLAGS,
               (const struct sockaddr *)&servaddr, len);

        int n = recvfrom(sockfd, buffer, (int)MAX_LINE, RECV_FLAGS,
                         (struct sockaddr *)&servaddr, &len);
        if (n <= 0) return;

        memcpy(players, buffer, n);
        for (int i = 0; i < n / (int)sizeof(payload); ++i)
        {
            engine->moveObject(scene_name, players[i].name, players[i].mat);
        }    
    }

    ~Mp() { 
#ifdef _WIN32
        if (sockfd != INVALID_SOCKET) {
            closesocket(sockfd);
        }
        WSACleanup();
#else
        if (sockfd >= 0) {
            close(sockfd);
        }
#endif
        free(players); 
    }

private:
#ifdef _WIN32
    SOCKET sockfd;
#else
    int sockfd;
#endif
    char buffer[MAX_LINE];
    struct sockaddr_in servaddr;
    socklen_t len;
    creation_payload crp;
    payload *players;
    Tiny_engine *engine;
    std::string scene_name;
};