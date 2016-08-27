#ifndef TUTORIALS_CONTROLS_H
#define TUTORIALS_CONTROLS_H


#include <SDL2/SDL_events.h>
#include <iostream>

class Controls {

  bool forward = false;
  bool backwards = false;
  bool left = false;
  bool right = false;
  bool up = false;
  bool down = false;

  float x = 0;
  float z = 0;
  float y = 0;

  bool leftMouse = false;
  bool rightMouse = false;

  bool jump = false;

  int blockType = 0;

public:

  void handleEvent(const SDL_Event& event) {
    if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
      switch(event.button.button) {
        case SDL_BUTTON_LEFT:
          leftMouse = event.button.state == SDL_PRESSED;
          break;
        case SDL_BUTTON_RIGHT:
          rightMouse = event.button.state == SDL_PRESSED;
          break;
      }
    }

    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
      switch (event.key.keysym.scancode) {
        case SDL_SCANCODE_W:
          forward = event.type == SDL_KEYDOWN;
          break;
        case SDL_SCANCODE_S:
          backwards = event.type == SDL_KEYDOWN;
          break;
        case SDL_SCANCODE_A:
          left = event.type == SDL_KEYDOWN;
          break;
        case SDL_SCANCODE_D:
          right = event.type == SDL_KEYDOWN;
          break;
        case SDL_SCANCODE_SPACE:
          jump = event.type == SDL_KEYDOWN;
          up = event.type == SDL_KEYDOWN;
          break;
        case SDL_SCANCODE_LSHIFT:
          down = event.type == SDL_KEYDOWN;
          break;
        default:
          break;
      }
    }
    if (event.type == SDL_KEYDOWN) {
      if (event.key.keysym.sym >= SDLK_1 && event.key.keysym.sym <= SDLK_9) {
        blockType = event.key.keysym.sym - SDLK_1;
      }
    }
  }

  void update() {
    x = z = y = 0;
    if (forward)
      z += 1;
    if (backwards)
      z -= 1;
    if (right)
      x += 1;
    if (left)
      x -= 1;
    if (up)
      y += 1;
    if (down)
      y -= 1;
  }

  int getBlockType() const {
    return blockType;
  }

  bool jumpPoll() {
    if (jump) {
      jump = false;
      return true;
    }
    return false;
  }

  bool leftMousePoll() {
    if (leftMouse) {
      leftMouse = false;
      return true;
    }
    return false;
  }

  bool rightMousePoll() {
    if (rightMouse) {
      rightMouse = false;
      return true;
    }
    return false;
  }

  float getX() const {
    return x;
  }

  float getZ() const {
    return z;
  }

  float getY() const {
    return y;
  }
};


#endif //TUTORIALS_CONTROLS_H
