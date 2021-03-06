#ifndef TUTORIALS_CAMERA_H
#define TUTORIALS_CAMERA_H


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_timer.h>
#include <iostream>
#include <common/Common.h>

class Camera {
  glm::mat4 ViewMatrix;
  glm::mat4 ProjectionMatrix;
// Initial position : on +Z
  glm::vec3 position = glm::vec3(0, 0, 0);
// Initial horizontal angle : toward -Z
  float horizontalAngle = 0;
// Initial vertical angle : none
  float verticalAngle = 0;
// Initial Field of View
  float initialFoV = 45.0f;

  float speed = 15.0f; // 3 units / second
  float mouseSpeed = 0.005f;
  Uint64 LAST = 0;

  glm::vec3 direction;

public:

  glm::mat4 getViewMatrix(){
    return ViewMatrix;
  }
  glm::mat4 getProjectionMatrix(){
    return ProjectionMatrix;
  }

  void setPos(float x, float y, float z) {

    Uint64 NOW = SDL_GetPerformanceCounter();
    float deltaTime = (((float) NOW - LAST) / SDL_GetPerformanceFrequency());
    LAST = NOW;

    // Direction : Spherical coordinates to Cartesian coordinates conversion
    direction = glm::vec3(
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

    position.x = x;
    position.y = y;
    position.z = z;


    float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

    // Projection matrix : 45� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    ProjectionMatrix = glm::perspective(FoV, 16.0f / 9.0f, 0.1f, 30000.0f);
    // Camera matrix
    ViewMatrix       = glm::lookAt(
      position,           // Camera is here
      position+direction, // and looks here : at the same position, plus "direction"
      up                  // Head is up (set to 0,-1,0 to look upside-down)
    );
  }

  void updatePos(float x, float y) {

    Uint64 NOW = SDL_GetPerformanceCounter();
    float deltaTime = (((float) NOW - LAST) / SDL_GetPerformanceFrequency());
    LAST = NOW;

    // Direction : Spherical coordinates to Cartesian coordinates conversion
    direction = glm::vec3(
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

  float getHorizAngle() {
    return horizontalAngle;
  }

  void handleEvent(const SDL_Event& event){
    //std::cout << position.x << ", " << position.y << ", " << position.z << "\n";
    //std::cout << horizontalAngle << " " << verticalAngle << std::endl;


    if (event.type == SDL_MOUSEMOTION) {
      // Compute new orientation
      horizontalAngle -= mouseSpeed * event.motion.xrel;
      verticalAngle   -= mouseSpeed * event.motion.yrel;

      verticalAngle = std::max(-PI / 2, std::min(verticalAngle, PI / 2));
    }

    //std::cout << "VANGLE: " << verticalAngle << std::endl;
  }

  glm::vec3 getDirection(float scale) const {
    return glm::vec3(direction.x * scale, direction.y * scale, direction.z * scale);
  }

  glm::vec3 getPosition() const {
    return position;
  }
};


#endif //TUTORIALS_CAMERA_H
