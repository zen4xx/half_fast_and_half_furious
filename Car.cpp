#include "Car.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

#define PI 3.1415

struct TorquePoint {
    double rpm;
    double torque;
};

float getEngineTorque(int rpm, float torque_multiplyer) {
    static const std::vector<TorquePoint> curve = {
        {800,  120},
        {1000, 150},
        {1500, 210},
        {2000, 260},
        {2500, 290},
        {3000, 310},
        {3500, 320},
        {4000, 320},
        {4500, 315},
        {5000, 300},
        {5500, 275},
        {6000, 245},
        {6500, 210},
        {7000, 160}
    };

    if (rpm <= curve.front().rpm)
        return curve.front().torque * torque_multiplyer;

    if (rpm >= curve.back().rpm)
        return curve.back().torque * torque_multiplyer;

    for (size_t i = 0; i < curve.size() - 1; i++) {
        const TorquePoint& p1 = curve[i];
        const TorquePoint& p2 = curve[i + 1];

        if (rpm >= p1.rpm && rpm <= p2.rpm) {
            float t =
                (rpm - p1.rpm) /
                (p2.rpm - p1.rpm);

            return p1.torque * torque_multiplyer +
                   t * (p2.torque * torque_multiplyer - p1.torque * torque_multiplyer);
        }
    }

    return 0.0;
}

float getEngineRPM(
    float speed,
    float wheelRadius,
    float gearRatio,
    float finalDrive
) {
    if (speed <= 0.0)
        return 1000.0; 

    float wheelRPM =
        speed / (2.0 * PI * wheelRadius) * 60.0;

    return wheelRPM * gearRatio * finalDrive;
}

void Car::accelerate(double dt)
{

    if (this->RPM >= this->max_RPM || this->gear == 1) return;

    float engineTorque =
        getEngineTorque(
            this->RPM,
            this->torque_multiplyer
        );

    float wheelTorque =
        engineTorque
        * this->gears[this->gear]
        * this->final_drive
        * this->efficiency;

    float tractionForce =
        wheelTorque / this->wheel_radius;

    float gripForce =
        this->friction
        * this->weight
        * this->driven_weight
        * g;

    float wheelForce =
        std::min(tractionForce, gripForce);

    float acceleration =
        wheelForce / this->weight;

    this->speed += acceleration * dt;
}

void Car::brake(double dt)
{
    if (this->speed > 0.0f)
    {
        float gripForce =
            this->friction
            * this->weight
            * g;

        float brakeForce =
            std::min(
                this->max_brake_force,
                gripForce
            );


        float totalForce =
            brakeForce
            + this->air_resistance * this->speed * this->speed
            + this->rolling_resistance;

        float deceleration =
            totalForce / this->weight;

        this->speed -= deceleration * dt;

        if (this->speed < 0.0f)
            this->speed = 0.0f;
    }

    if (this->speed < 0.0f)
    {
        float gripForce =
            this->friction
            * this->weight
            * g;

        float brakeForce =
            std::min(
                this->max_brake_force,
                gripForce
            );


        float totalForce =
            brakeForce
            + this->air_resistance * this->speed * this->speed
            + this->rolling_resistance;

        float deceleration =
            totalForce / this->weight;

        this->speed += deceleration * dt;

        if (this->speed > 0.0f)
            this->speed = 0.0f;
    }
}


void Car::update(double dt)
{
    this->RPM = getEngineRPM(this->speed, this->wheel_radius, this->gears[this->gear], this->final_drive);

    if (this->speed != 0)
    {
        this->speed -= (this->rolling_resistance + this->air_resistance * speed * speed) / this->weight * dt;

        m_front.x = sin(glm::radians(m_yaw));
        m_front.y = 0;
        m_front.z = cos(glm::radians(m_yaw));

        
        m_front = glm::normalize(m_front);
        m_right = glm::normalize(glm::cross(m_front, m_world_up));

        m_pos += m_front * float(this->speed * dt);

        m_engine->moveObject(m_scene_name, m_obj_name, glm::rotate(glm::translate(m_original_rotation, m_pos), glm::radians(m_yaw), glm::vec3(0.f, 1.f, 0.f)));
    }
}

void Car::turn(dir d, double dt)
{

    if (this->speed != 0)
    {
        if (d == right)
        {
            this->m_yaw += this->turn_angle * dt;
        }
        else if (d == left)
        {
            this->m_yaw -= this->turn_angle * dt;
        }
    }
}
