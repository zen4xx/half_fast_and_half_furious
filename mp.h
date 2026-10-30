// multiplayer
#include "tiny_engine.h"
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <vector>
#include <map>

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

struct payload {
    int index;
    char name[NAME_LEN];
    glm::mat4 mat;
    glm::vec3 hpos; // helmet pos (if gamemode is 1)
};

struct creation_payload {
    int index = -1;
    char name[NAME_LEN];
    char gltf[CREATION_LEN];
    char start = 0; 
    char game_mode = 0; // 0 is without any modes, 1 is catch the helmet
};

class Mp {
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

    void set_player(const char name[NAME_LEN], const char gltf[CREATION_LEN], std::string server_ip, Tiny_engine *engine, std::string scene_name, char game_mode) {
        strcpy(crp.name, name);
        strcpy(crp.gltf, gltf);
        crp.game_mode = game_mode;

        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            perror("socket creation failed");
            exit(EXIT_FAILURE);
        }
        memset(&servaddr, 0, sizeof(servaddr));
        
        struct timeval timeout = { .tv_sec = 1, .tv_usec = 0 };
        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) == -1) {
            perror("setsockopt(SO_RCVTIMEO)");
            CLOSE_SOCKET(sockfd);
            exit(EXIT_FAILURE);
        }

        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(PORT);
        servaddr.sin_addr.s_addr = inet_addr(server_ip.c_str());
        len = sizeof(servaddr);

        sendto(sockfd, (const char*)&crp, (int)sizeof(crp), SEND_FLAGS, (const struct sockaddr *)&servaddr, len);

        int n = recvfrom(sockfd, buffer, (int)MAX_LINE, RECV_FLAGS, (struct sockaddr *)&servaddr, &len);
        if (n <= 0) {
            std::cerr << "Failed to receive response from server" << std::endl;
            CLOSE_SOCKET(sockfd);
            exit(EXIT_FAILURE);
        }

        crp.index = *(int*)buffer;
        this->engine = engine;
        this->scene_name = scene_name;
    }

    void start() {
        crp.start = 1;
        memset(buffer, 0, MAX_LINE);

        sendto(sockfd, (const char*)&crp, (int)sizeof(crp), SEND_FLAGS, (const struct sockaddr *)&servaddr, len);

        int n = recvfrom(sockfd, buffer, (int)MAX_LINE, RECV_FLAGS, (struct sockaddr *)&servaddr, &len);
        if (n <= 0) return;

        creation_payload *all_crps = (creation_payload*)malloc(n);
        memcpy(all_crps, buffer, n);

        int num_players = n / (int)sizeof(creation_payload);
        for (int i = 0; i < num_players; ++i) {
            tiny_engine::Object obj;
            obj.scene_name = this->scene_name;
            obj.pos = glm::mat4(1);
            obj.gltf_model_path = all_crps[i].gltf;

            size_t obj_id = engine->addObject(obj);
            index_to_obj_id[all_crps[i].index] = obj_id;
            
            if (all_crps[i].index == crp.index) {
                local_player_obj_id = obj_id;
            }
        }

        if (crp.game_mode == 1) { // catch the helmet
            helmet_obj_id = engine->addObject(
                this->scene_name, 
                "damaged_helmet/DamagedHelmet.gltf", 
                glm::rotate(glm::mat4(1), glm::radians(-90.f), glm::vec3(1, 0, 0)), 
                "damaged_helmet/Default_albedo.jpg", 
                "damaged_helmet/Default_metalRoughness.jpg", 
                "damaged_helmet/Default_normal.jpg"
            );
        }

        players = (payload*)malloc(num_players * sizeof(payload));
        free(all_crps);
    }

    void update(glm::mat4 mat) {
        payload p;
        p.mat = mat;
        p.index = crp.index;        
        memcpy(p.name, crp.name, NAME_LEN);
        
        sendto(sockfd, (const char*)&p, (int)sizeof(p), SEND_FLAGS, (const struct sockaddr *)&servaddr, len);

        int n = recvfrom(sockfd, buffer, (int)MAX_LINE, RECV_FLAGS, (struct sockaddr *)&servaddr, &len);
        if (n <= 0) return;

        memcpy(players, buffer, n);
        int num_players = n / (int)sizeof(payload);
        
        for (int i = 0; i < num_players; ++i) {
            auto it = index_to_obj_id.find(players[i].index);
            if (it != index_to_obj_id.end()) {
                engine->moveObject(scene_name, it->second, players[i].mat);
            }
        }    

        if (crp.game_mode == 1 && helmet_obj_id != 0) {
            glm::mat4 hmodel(1.0f);
            hmodel = glm::translate(hmodel, players[0].hpos);
            hmodel = glm::rotate(hmodel, glm::radians(180.f), glm::vec3(0, 0, 1));
            hmodel = glm::rotate(hmodel, glm::radians((float)glfwGetTime() * 50.f), glm::vec3(0, 1, 0));
            engine->moveObject(scene_name, helmet_obj_id, hmodel);
        }
    }

    size_t get_local_player_obj_id() const { return local_player_obj_id; }

    ~Mp() { 
        if (sockfd >= 0) CLOSE_SOCKET(sockfd);
        free(players); 
    }

private:
    int sockfd;
    char buffer[MAX_LINE];
    struct sockaddr_in servaddr;
    socklen_t len;
    creation_payload crp;
    payload *players;
    
    std::map<int, size_t> index_to_obj_id;
    size_t local_player_obj_id = 0;
    size_t helmet_obj_id = 0;
    Tiny_engine *engine;
    std::string scene_name;
};