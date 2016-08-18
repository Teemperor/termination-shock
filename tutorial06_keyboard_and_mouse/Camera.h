#ifndef TUTORIALS_CAMERA_H
#define TUTORIALS_CAMERA_H


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_timer.h>
#include <iostream>

class Camera {
  glm::mat4 ViewMatrix;
  glm::mat4 ProjectionMatrix;
// Initial position : on +Z
  glm::vec3 position = glm::vec3(-4.92819, 68.359, -3.56799);
// Initial horizontal angle : toward -Z
  float horizontalAngle = -5.31364f;
// Initial vertical angle : none
  float verticalAngle = -7.03576f;
// Initial Field of View
  float initialFoV = 45.0f;

  float speed = 15.0f; // 3 units / second
  float mouseSpeed = 0.005f;
  Uint64 LAST = 0;

public:

  glm::mat4 getViewMatrix(){
    return ViewMatrix;
  }
  glm::mat4 getProjectionMatrix(){
    return ProjectionMatrix;
  }

  void updatePos(float x, float y) {

    Uint64 NOW = SDL_GetPerformanceCounter();
    float deltaTime = (((float) NOW - LAST) / SDL_GetPerformanceFrequency());
    LAST = NOW;

    // Direction : Spherical coordinates to Cartesian coordinates conversion
    glm::vec3 direction(
      cos(verticalAngle) * sin(horizontalAngle),
      sin(verticalAngle),
      cos(verticalAngle) * cos(horizontalAngle)
    );

    // Right vector
    glm::vec3 right = glm::vec3(
      sin(horizontalAngle - 3.14f/2.0f),
      0,
      cos(horizontalAngle - 3.14f/2.0f)
    );

    // Up vector
    const glm::vec3 up = glm::cross( right, direction );

    position += x * right * deltaTime * speed;
    position += y * direction * deltaTime * speed;


    float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

    // Projection matrix : 45� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    ProjectionMatrix = glm::perspective(FoV, 4.0f / 3.0f, 0.1f, 30000.0f);
    // Camera matrix
    ViewMatrix       = glm::lookAt(
      position,           // Camera is here
      position+direction, // and looks here : at the same position, plus "direction"
      up                  // Head is up (set to 0,-1,0 to look upside-down)
    );

  }

  void handleEvent(const SDL_Event& event){
    //std::cout << position.x << ", " << position.y << ", " << position.z << "\n";
    //std::cout << horizontalAngle << " " << verticalAngle << std::endl;


    if (event.type == SDL_MOUSEMOTION) {
      // Compute new orientation
      horizontalAngle -= mouseSpeed * event.motion.xrel;
      verticalAngle   -= mouseSpeed * event.motion.yrel;
    }
  }
};


#endif //TUTORIALS_CAMERA_H
