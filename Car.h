#pragma once

#include "tiny_engine.h"
#include <vector>
#include <string>

enum dir{
    right,
    left,
};

class Car 
{
public:
    inline void set(const std::string &scene_name, const std::string &obj_name, Tiny_engine *engine, glm::vec3 world_up, glm::mat4 rotation, glm::vec3 pos = glm::vec3(0)) 
    {
        m_scene_name = scene_name;
        m_obj_name = obj_name;
        m_engine = engine; 
        m_pos = pos;
        m_world_up = world_up;
        m_original_rotation = rotation;
    };

    void accelerate(double dt);
    void brake(double dt);
    void engine_brake(double dt);
    void turn(dir d, double dt);

    void update(double dt);

public:
    float torque_multiplyer = 1;
    int weight = 1000; // kg
    float max_RPM = 7000;
    int gears_num = 7; // R + N + 5
    
    float driven_weight = weight * 0.6;

    float friction = 0.90; 
    float wheel_radius = 0.254; // m 

    float g = 9.81; // m/s^2

    float frontal_area = 2.2;
    float efficiency = 0.90;

    float final_drive = 4.1;

    float max_brake_force = 15000.f; // N

    float air_resistance = frontal_area * 1.225 * 0.5 * 0.3;
    float rolling_resistance = 0.015 * weight * g;

    std::vector<float> gears = {
        -4,   // R
        255,  // N
        3.50, // 1
        2.10, // 2
        1.40, // 3
        1.00, // 4
        0.80  // 5
    };
    
    float speed = 0.f; //m/s
    
    float RPM = 1000;
    
    int gear = 2; // 1 is N (cause 0 is R)

    float turn_angle = 70;
    
public:
    Tiny_engine *m_engine = nullptr;
    std::string m_scene_name = "", m_obj_name = "";
    
    glm::vec3 m_pos = glm::vec3(0);
    float m_yaw = 0.f;
    glm::vec3 m_front, m_right, m_world_up;

    glm::mat4 m_original_rotation;

};