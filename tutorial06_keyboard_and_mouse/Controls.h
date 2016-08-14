#ifndef TUTORIALS_CONTROLS_H
#define TUTORIALS_CONTROLS_H


#include <SDL2/SDL_events.h>
#include <iostream>

class Controls {

  bool up = false;
  bool down = false;
  bool left = false;
  bool right = false;

  float x = 0;
  float y = 0;

public:

  void handleEvent(const SDL_Event& event) {
    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
      switch (event.key.keysym.sym) {
        case SDLK_w:
          up = event.type == SDL_KEYDOWN;
          break;
        case SDLK_s:
          down = event.type == SDL_KEYDOWN;
          break;
        case SDLK_a:
          left = event.type == SDL_KEYDOWN;
          break;
        case SDLK_d:
          right = event.type == SDL_KEYDOWN;
          break;
      }
    }
  }

  void update() {
    x = y = 0;
    if (up)
      y += 1;
    if (down)
      y -= 1;
    if (right)
      x += 1;
    if (left)
      x -= 1;
  }

  float getX() const {
    return x;
  }

  float getY() const {
    return y;
  }
};


#endif //TUTORIALS_CONTROLS_H
