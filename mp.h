// multiplayer
#include "renderer/renderer.h"
#include "tiny_engine.h"
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>

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
    void set_player(char name[NAME_LEN], char gltf[CREATION_LEN], std::string server_ip, Tiny_engine *engine, std::string scene_name)
    {
        memcpy(crp.name, name, NAME_LEN);
        memcpy(crp.gltf, gltf, CREATION_LEN);

        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0)
        {
            perror("socket creation failed");
            exit(EXIT_FAILURE);
        }
        memset(&servaddr, 0, sizeof(servaddr));
        
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(PORT);
        servaddr.sin_addr.s_addr = inet_addr(server_ip.c_str());

        len = sizeof(servaddr);

        sendto(sockfd, &crp, sizeof(crp), MSG_CONFIRM,
        (const struct sockaddr *)&servaddr, sizeof(servaddr));

        int n = recvfrom(sockfd, buffer, MAX_LINE, MSG_WAITALL,
                            (struct sockaddr *)&servaddr, &len);

        crp.index = *(int*)buffer;
        this->engine = engine;
        this->scene_name = scene_name;
    }

    void start()
    {
        crp.start = 1;
        memset(buffer, 0, MAX_LINE);
        std::cout << "sendto\n";
        sendto(sockfd, &crp, sizeof(crp), MSG_CONFIRM,
        (const struct sockaddr *)&servaddr, sizeof(servaddr));

        int n = recvfrom(sockfd, buffer, MAX_LINE, MSG_WAITALL,
                      (struct sockaddr *)&servaddr, &len);
        std::cout << "recv\n";
        creation_payload *all_crps = (creation_payload*)malloc(n);
        memcpy(all_crps, buffer, n);

        for (int i = 0; i < n / sizeof(creation_payload); ++i)
        {
            std::cout << "obj\n";
            tiny_engine::Object obj;
            obj.obj_name = all_crps[i].name;
            obj.scene_name = this->scene_name;
            obj.pos = glm::mat4(1);
            obj.gltf_model_path = all_crps[i].gltf;

            engine->addObject(obj);
        }

        free(all_crps);
    };

    void update(glm::mat4 mat)
    {
        payload p;
        p.mat = mat;
        p.index = crp.index;        
        memcpy(p.name, crp.name, NAME_LEN);
        
        sendto(sockfd, &p, sizeof(p), MSG_CONFIRM,
        (const struct sockaddr *)&servaddr, sizeof(servaddr));
        memset(buffer, 0, MAX_LINE);
        int n = recvfrom(sockfd, buffer, MAX_LINE, MSG_WAITALL,
            (struct sockaddr *)&servaddr, &len);
        payload* players = (payload*)malloc(n);
        memcpy(players, buffer, n);
        for (int i = 0; i < n / sizeof(payload); ++i)
        {
            engine->moveObject(scene_name, players[i].name, players[i].mat);
        }    
        free(players);
        
    }

    ~Mp() { close(sockfd); };

private:
    int sockfd;
    char buffer[MAX_LINE];
    struct sockaddr_in servaddr;
    socklen_t len;
    creation_payload crp;
    Tiny_engine *engine;
    std::string scene_name;
};