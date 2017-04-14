#ifndef TUTORIALS_RENDERWINDOW_H
#define TUTORIALS_RENDERWINDOW_H


// fixed MINGW error http://stackoverflow.com/questions/5259714/undefined-reference-to-winmain16
#define SDL_MAIN_HANDLED

#include <GL/glew.h>
#include <SDL2/SDL.h>
//#include <SDL2/SDL_opengl.h>
#include <iostream>

class RenderWindow {


  SDL_Window *Window;
  //OpenGL context
  SDL_GLContext Context;

  int Width, Height;

  void close() {
    //Destroy window
    SDL_DestroyWindow(Window);
    Window = NULL;

    SDL_GL_DeleteContext(Context);

    //Quit SDL subsystems
    SDL_Quit();
  }


public:
  RenderWindow(int w, int h) : Width(w), Height(h) {
    if (!init()) {
      std::cerr << "Init failure" << std::endl;
      exit(1);
    }
  }

  ~RenderWindow() {
    close();
  }

  bool init() {
    //Initialization flag
    bool success = true;

    //Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
      success = false;
    }
    else {
      //Use OpenGL version
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

      SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
      SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);

      SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

      //Create window
      Window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED, Width, Height,
                                SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP);
      if (Window == NULL) {
        printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
        success = false;
      }
      else {
        //Create context
        Context = SDL_GL_CreateContext(Window);
        if (Context == NULL) {
          printf("OpenGL context could not be created! SDL Error: %s\n",
                 SDL_GetError());
          success = false;
        }
        else {
          //Use Vsync
          //if (SDL_GL_SetSwapInterval(1) < 0) {
          //  printf("Warning: Unable to set VSync! SDL Error: %s\n",
          //         SDL_GetError());
          //}

          //Initialize OpenGL
          if (!initGL()) {
            printf("Unable to initialize OpenGL!\n");
            success = false;
          }
        }
        // Grab the mouse
        SDL_SetRelativeMouseMode(SDL_TRUE);
      }
    }

    return success;
  }

  bool initGL() {
    bool success = true;
    GLenum error = GL_NO_ERROR;

    //Initialize clear color
    glClearColor(0.f, 0.f, 0.f, 1.f);

    //Check for error
    error = glGetError();
    if (error != GL_NO_ERROR) {
      success = false;
    }

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
      fprintf(stderr, "Failed to initialize GLEW\n");
      exit(1);
    }


    return success;
  }

  void swap() {
    SDL_GL_SwapWindow(Window);
  }

};


#endif //TUTORIALS_RENDERWINDOW_H
