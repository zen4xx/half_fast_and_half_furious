#include "renderer/renderer.h"
#include "renderer/renderer_util.h"
#include "tiny_engine.h"
#include "camera.h"
#include "Car.h"
#include <glm/ext/matrix_transform.hpp>
#include <string>
#include <thread>
#include <iostream>
#include "sound.h"
#include "mp.h"

#define GAME_NAME "half fast & half furious"

Camera camera;

double lastX = 400, lastY = 300;
bool firstMouse = true;
 
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
     if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
 
    lastX = xpos;
    lastY = ypos;
  
    camera.ProcessMouseMovement(xoffset, yoffset);
}

int main()
{
    uint8_t thread_count = std::thread::hardware_concurrency();
    
    auto engine = Tiny_engine(GAME_NAME, 1920, 1080, GAME_NAME, TINY_ENGINE_MAX_MSAA_QUALITY, thread_count, true);

    glfwSetCursorPosCallback(engine.getWindow(), mouse_callback);
    glfwSetInputMode(engine.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(engine.getWindow(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    engine.createScene("main");
    engine.setDirLight("main", glm::vec3(2.0f, 0.5f, 2.0f));
    engine.setDrawDistance("main", 100.f);
    engine.setView("main", camera.GetViewMatrix());

    tiny_engine::Object plane;
    plane.scene_name = "main";
    plane.obj_name = "plane";
    plane.albedo_path = "plane/TwoSidedPlane_BaseColor.png";
    plane.mr_path = "plane/TwoSidedPlane_MetallicRoughness.png";
    plane.normal_path = "plane/TwoSidedPlane_Normal.png";
    plane.gltf_model_path = "plane/TwoSidedPlane.gltf";
    plane.pos = glm::scale(glm::rotate(glm::mat4(1), glm::radians(-180.f), glm::vec3(0.f, 0.f, 1.f)), glm::vec3(50.f));

    tiny_engine::Object mustang;

    engine.addObject(plane);

    Mp mp;
    bool is_mp;
    std::cout << "is multiplayer 1/0: ";
    std::cin >> is_mp;

    Car car;
    
    if (is_mp == 0){
        mustang.scene_name = "main";
        mustang.obj_name = "mustang";
        mustang.gltf_model_path = "cars/mustang/mustang.gltf";
        mustang.pos = glm::translate(glm::mat4(1), glm::vec3(0.f));

        engine.addObject(mustang);
        car.set("main", "mustang", &engine, glm::vec3(0.0f, 1.0f, 0.0f), glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(1.0f, 0.0f, 0.0f)));
    }

    else 
    {
        std::string ip, name;
        std::cout << "enter server ip: ";
        std::cin >> ip;
        std::cout << "enter your name: ";
        std::cin >> name;
        
        mp.set_player(name.c_str(), (char*)"cars/mustang/mustang.gltf", ip, &engine, "main");
    
        car.set("main", name, &engine, glm::vec3(0.0f, 1.0f, 0.0f), glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(1.0f, 0.0f, 0.0f)));

        std::cout << "write something to start\n";
        std::cin >> ip;
        mp.start();        
    }
    
    car.torque_multiplyer = 2.64f;
    car.weight = 1900.f;

    bool key_1 = 0, key_2 = 0, free_cam = 0, third_person_cam = 0; // toggles

    bool is_free_cam = 0;

    glm::vec3 car_tail_pos = glm::vec3(0);

    Sound_engine sound_engine;
    sound_engine.start();

    while (engine.isWindowOpen()){
        if (engine.isKeyPressed(GLFW_KEY_W))
            camera.ProcessKeyboard(GLFW_KEY_W, engine.getDeltaTime());
        if (engine.isKeyPressed(GLFW_KEY_S))
            camera.ProcessKeyboard(GLFW_KEY_S, engine.getDeltaTime());
        if (engine.isKeyPressed(GLFW_KEY_D))
            camera.ProcessKeyboard(GLFW_KEY_D, engine.getDeltaTime());
        if (engine.isKeyPressed(GLFW_KEY_A))
            camera.ProcessKeyboard(GLFW_KEY_A, engine.getDeltaTime());

        if (engine.isKeyPressed(GLFW_KEY_F)) std::cout << engine.getFPSCount() << std::endl;

        if (engine.isKeyPressed(GLFW_KEY_UP)) { car.accelerate(engine.getDeltaTime()); sound_engine.setThrottle(1.f); }
        else { car.engine_brake(engine.getDeltaTime()); sound_engine.setThrottle(0.f); }
        if (engine.isKeyPressed(GLFW_KEY_DOWN)) car.brake(engine.getDeltaTime());
        if (engine.isKeyPressed(GLFW_KEY_RIGHT)) car.turn(right, engine.getDeltaTime());
        if (engine.isKeyPressed(GLFW_KEY_LEFT)) car.turn(left, engine.getDeltaTime());

        if (engine.isKeyPressed(GLFW_KEY_1)) key_1 = 1;
        else if (!engine.isKeyPressed(GLFW_KEY_1) && key_1 == 1) {key_1 = 0; car.gear--; }
        if (engine.isKeyPressed(GLFW_KEY_2)) key_2 = 1;
        else if (!engine.isKeyPressed(GLFW_KEY_2) && key_2 == 1) {key_2 = 0; car.gear++; }
        if (engine.isKeyPressed(GLFW_KEY_C)) free_cam = 1;
        else if (!engine.isKeyPressed(GLFW_KEY_C) && free_cam == 1) {free_cam = 0; is_free_cam = 1; camera.Position = glm::vec3(car_tail_pos.x, car_tail_pos.y - 1.5f, -car_tail_pos.z); camera.Yaw = car.m_yaw - 90.f; camera.updateCameraVectors(); }
        if (engine.isKeyPressed(GLFW_KEY_X)) third_person_cam = 1;
        else if (!engine.isKeyPressed(GLFW_KEY_X) && third_person_cam == 1) {third_person_cam = 0; is_free_cam = 0; }
        
        
        std::cout << "RPM: " << car.RPM << std::endl;
        std::cout << "speed km/h: " << car.speed * 3.6 << std::endl;
        std::cout << "gear: " << car.gear - 1 << std::endl;
        
        car.update(engine.getDeltaTime());

        if (is_free_cam)
            engine.setView("main", camera.GetViewMatrix());

        else 
        {
            if (car.gear != 0)
                car_tail_pos = car.m_pos - (car.m_front * 3.f * ((car.speed/50)+1.f));
            else 
                car_tail_pos = car.m_pos - (car.m_front * -5.f * ((-car.speed/50)+1.f)); // reverse

            engine.setView("main", glm::lookAt(glm::vec3(car_tail_pos.x, car_tail_pos.y - 1.5f, -car_tail_pos.z), glm::vec3(car.m_pos.x, car.m_pos.y - 1.f, -car.m_pos.z), camera.WorldUp));
        }
        
        if (is_mp)
            mp.update(glm::rotate(glm::translate(glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(1.0f, 0.0f, 0.0f)), car.m_pos), glm::radians(car.m_yaw), glm::vec3(0.f, 1.f, 0.f)));
        sound_engine.setRPM(car.RPM);
        engine.update();
        engine.drawScene("main");
    }

}
