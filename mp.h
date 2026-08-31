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
    void set_player(const char name[NAME_LEN], const char gltf[CREATION_LEN], std::string server_ip, Tiny_engine *engine, std::string scene_name)
    {
        strcpy(crp.name, name);
        strcpy(crp.gltf, gltf);

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

        recvfrom(sockfd, buffer, MAX_LINE, MSG_WAITALL,
                            (struct sockaddr *)&servaddr, &len);

        crp.index = *(int*)buffer;
        this->engine = engine;
        this->scene_name = scene_name;
    }

    void start()
    {
        crp.start = 1;
        memset(buffer, 0, MAX_LINE);

        sendto(sockfd, &crp, sizeof(crp), MSG_CONFIRM,
        (const struct sockaddr *)&servaddr, sizeof(servaddr));

        int n = recvfrom(sockfd, buffer, MAX_LINE, MSG_WAITALL,
                      (struct sockaddr *)&servaddr, &len);

        creation_payload *all_crps = (creation_payload*)malloc(n);
        memcpy(all_crps, buffer, n);

        for (int i = 0; i < n / (int)sizeof(creation_payload); ++i)
        {
            std::cout << i << std::endl;
            tiny_engine::Object obj;
            obj.obj_name = all_crps[i].name;
            obj.scene_name = this->scene_name;
            obj.pos = glm::mat4(1);
            obj.gltf_model_path = all_crps[i].gltf;

            engine->addObject(obj);
        }
        players = (payload*)malloc(n/sizeof(creation_payload) * sizeof(payload));
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

        int n = recvfrom(sockfd, buffer, MAX_LINE, MSG_WAITALL,
            (struct sockaddr *)&servaddr, &len);
        memcpy(players, buffer, n);
        for (int i = 0; i < n / (int)sizeof(payload); ++i)
        {
            engine->moveObject(scene_name, players[i].name, players[i].mat);
        }    
        
    }

    ~Mp() { close(sockfd); free(players); };

private:
    int sockfd;
    char buffer[MAX_LINE];
    struct sockaddr_in servaddr;
    socklen_t len;
    creation_payload crp;
    payload *players;
    Tiny_engine *engine;
    std::string scene_name;
};