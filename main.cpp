#include "renderer/renderer.h"
#include "renderer/renderer_util.h"
#include "tiny_engine.h"
#include "camera.h"
#include "Car.h"
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/trigonometric.hpp>
#include <thread>
#include <iostream>

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
    mustang.scene_name = "main";
    mustang.obj_name = "mustang";
    mustang.gltf_model_path = "cars/mustang/mustang.gltf";
    mustang.pos = glm::translate(glm::mat4(1), glm::vec3(0.f));

    engine.addObject(plane);
    engine.addObject(mustang);

    Car car;

    car.set("main", "mustang", &engine, glm::vec3(0.0f, 1.0f, 0.0f), glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(1.0f, 0.0f, 0.0f)));
    
    bool key_1 = 0, key_2 = 0; // toggles

    glm::vec3 car_tail_pos = glm::vec3(0);

    while (engine.isWindowOpen()){

        if (engine.isKeyPressed(GLFW_KEY_W)) camera.Pos.z -= 0.001;
        if (engine.isKeyPressed(GLFW_KEY_S)) camera.Pos.z += 0.001;
        if (engine.isKeyPressed(GLFW_KEY_A)) camera.Pos.x -= 0.001;
        if (engine.isKeyPressed(GLFW_KEY_D)) camera.Pos.x += 0.001;
        if (engine.isKeyPressed(GLFW_KEY_Z)) camera.Pos.y += 0.001;
        if (engine.isKeyPressed(GLFW_KEY_X)) camera.Pos.y -= 0.001;

        if (engine.isKeyPressed(GLFW_KEY_F)) std::cout << engine.getFPSCount() << std::endl;

        if (engine.isKeyPressed(GLFW_KEY_UP)) car.accelerate(engine.getDeltaTime());
        if (engine.isKeyPressed(GLFW_KEY_DOWN)) car.brake(engine.getDeltaTime());
        if (engine.isKeyPressed(GLFW_KEY_RIGHT)) car.turn(right, engine.getDeltaTime());
        if (engine.isKeyPressed(GLFW_KEY_LEFT)) car.turn(left, engine.getDeltaTime());

        if (engine.isKeyPressed(GLFW_KEY_1)) key_1 = 1;
        else if (!engine.isKeyPressed(GLFW_KEY_1) && key_1 == 1) {key_1 = 0; car.gear--; }
        if (engine.isKeyPressed(GLFW_KEY_2)) key_2 = 1;
        else if (!engine.isKeyPressed(GLFW_KEY_2) && key_2 == 1) {key_2 = 0; car.gear++; }
        
        
        std::cout << "RPM: " << car.RPM << std::endl;
        std::cout << "speed km/h: " << car.speed * 3.6 << std::endl;
        std::cout << "gear: " << car.gear - 1 << std::endl;
        
        car.update(engine.getDeltaTime());
        // car_tail_pos = -car.m_pos - (car.m_front * -3.f);
        // engine.setView("main", glm::lookAt(glm::vec3(car_tail_pos.x, car_tail_pos.y - 1, car_tail_pos.z), -car.m_pos, camera.WorldUp));
        engine.setView("main", camera.GetViewMatrix());
        engine.update();
        engine.drawScene("main");
    }

}
