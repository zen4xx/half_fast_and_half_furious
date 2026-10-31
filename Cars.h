#pragma once
#include "Car.h"
#include "renderer/renderer.h"
#include <string>

inline tiny_engine::Object create_mustang_obj(std::string scene_name)
{
    tiny_engine::Object mustang;
    mustang.gltf_model_path = "cars/mustang/mustang.gltf";
    mustang.scene_name = scene_name;
    mustang.pos = glm::mat4(1);

    return mustang;
}

inline tiny_engine::Object create_ferrari_f40_obj(std::string scene_name)
{
    tiny_engine::Object ferrari;
    ferrari.gltf_model_path = "cars/ferrari_f40/ferrari_f40.gltf";
    ferrari.albedo_path = "cars/ferrari_f40/ferrari_albedo.png";
    ferrari.mr_path = "cars/ferrari_f40/ferrari_mr.png";
    ferrari.normal_path = "cars/ferrari_f40/ferrari_nrm.png";
    ferrari.scene_name = scene_name;
    ferrari.pos = glm::mat4(1);

    return ferrari;
}

inline tiny_engine::Object create_diablo_obj(std::string scene_name)
{
    tiny_engine::Object diablo;
    diablo.gltf_model_path = "cars/lamborgambr_diablo/lambrgambr_diablo.gltf";
    diablo.albedo_path = "cars/lamborgambr_diablo/diablo_albedo.png";
    diablo.normal_path = "cars/lamborgambr_diablo/diablo_nrm.png";
    diablo.mr_path = "cars/lamborgambr_diablo/diablo_mr.png";
    diablo.pos = glm::mat4(1);
    diablo.scene_name = scene_name;
    return diablo;
}

inline tiny_engine::Object create_porsche_911_obj(std::string scene_name)
{
    tiny_engine::Object porsche;
    porsche.gltf_model_path = "cars/porsche_911/porsche.gltf";
    porsche.albedo_path = "cars/porsche_911/porsche_albedo.png";
    porsche.normal_path = "cars/porsche_911/porsche_nrm.png";
    porsche.mr_path = "cars/porsche_911/porsche_mr.png";
    porsche.pos = glm::mat4(1);
    porsche.scene_name = scene_name;
    return porsche;
}

inline tiny_engine::Object create_supra_obj(std::string scene_name)
{
    tiny_engine::Object supra;
    supra.gltf_model_path = "cars/supra/supra.gltf";
    supra.albedo_path = "cars/supra/supra_albedo.png";
    supra.normal_path = "cars/supra/supra_nrm.png";
    supra.mr_path = "cars/supra/supra_mr.png";
    supra.pos = glm::mat4(1);
    supra.scene_name = scene_name;
    return supra;
}

inline tiny_engine::Object create_car_obj(int car_index, std::string scene_name)
{
    tiny_engine::Object car;

    switch (car_index)
    {
        case(0):
        {
            car = create_mustang_obj(scene_name);
            break;
        }
        case(1):
        {
            car = create_ferrari_f40_obj(scene_name);
            break;
        }
        case(2):
        {
            car = create_diablo_obj(scene_name);
            break;
        }
        case(3):
        {
            car = create_porsche_911_obj(scene_name);
            break;
        }
        case(4):
        {
            car = create_supra_obj(scene_name);
            break;
        }
        default:
        {
            car = create_mustang_obj(scene_name);
            break;
        }
    }

    return car;
}

inline void set_mustang(Car& car)
{
    car.weight = 1900.f;
    car.torque_multiplyer = 2.64f;
    car.gears = {
        -3.15f, // R
        255.f,  // N
        3.35f,  // 1
        1.93f,  // 2
        1.29f,  // 3
        1.00f,  // 4
        0.68f   // 5
    };
    car.turn_angle = 70;
}

inline void set_ferrari_f40(Car& car)
{
    car.weight = 1254.f;
    car.torque_multiplyer = 1.8f;
    car.gears = {
        -2.90f, // R
        255.f,  // N
        2.77f,  // 1
        1.71f,  // 2
        1.23f,  // 3
        0.96f,  // 4
        0.76f   // 5
    };
    car.turn_angle = 95;
}

inline void set_diablo(Car& car)
{
    car.weight = 1600.f;
    car.torque_multiplyer = 1.89f;
    car.gears = {
        -2.12f, // R
        255.f,  // N
        2.31f,  // 1
        1.52f,  // 2
        1.12f,  // 3
        0.88f,  // 4
        0.68f   // 5
    };
    car.turn_angle = 80;
}

inline void set_porsche_911(Car& car)
{
    car.weight = 1580.f;
    car.torque_multiplyer = 1.84f;
    car.gears = {
        -3.60f, // R
        255.f,  // N
        3.50f,  // 1
        2.059f, // 2
        1.407f, // 3
        1.086f, // 4
        0.868f  // 5
    };
    car.turn_angle = 87;
}

inline void set_supra(Car& car)
{
    car.weight = 1225.f;
    car.torque_multiplyer = 3.1f;
    car.gears = {
        -3.28f, // R
        255.f,  // N
        3.827f, // 1
        2.360f, // 2
        1.685f, // 3
        1.312f, // 4
        1.000f, // 5
        0.793f  // 6
    };
    car.turn_angle = 65;
    car.max_brake_force = 25000;
}


inline void set_car(int car_index, Car &car)
{
    switch (car_index)
    {
        case(0):
        {
            set_mustang(car);
            break;
        }
        case(1):
        {
            set_ferrari_f40(car);
            break;
        }
        case(2):
        {
            set_diablo(car);
            break;
        }
        case(3):
        {
            set_porsche_911(car);
            break;
        }
        case(4):
        {
            set_supra(car);
            break;
        }
        default:
        {
            set_mustang(car);
            break;
        }
    }
}