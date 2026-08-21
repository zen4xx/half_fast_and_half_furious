#pragma once

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>

class Camera
{
public: 
    glm::vec3 Pos;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float Yaw;
    float Pitch;

    float MouseSensitivity;

    Camera(glm::vec3 pos = glm::vec3(0.f, -1.f, 3.f), glm::vec3 up = glm::vec3(0.f, 1.f, 0.f), float yaw = -90.f, float pitch = 0.f)
     : Front(glm::vec3(0.f, 0.f, -1.f)), 
       MouseSensitivity(0.03f)
    {

        Pos = pos;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    inline glm::mat4 GetViewMatrix() {
        return glm::lookAt(Pos, Pos + Front, Up);
    }
    
    inline void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;
             
        Yaw   += xoffset;
        Pitch -= yoffset;
             
        if (constrainPitch) {
            if (Pitch > 89.0f)  Pitch = 89.0f;
            if (Pitch < -89.0f) Pitch = -89.0f;
        }

        updateCameraVectors();
    }

private:
    inline void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
    
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up    = glm::normalize(glm::cross(Right, Front));
      
    }
};