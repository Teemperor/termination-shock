// Include standard headers
#include <stdio.h>
#include <stdlib.h>

#include "RenderWindow.h"
#include "FPSCounter.h"

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <vector>
#include <memory>
#include <chrono>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <map>
#include <cstring>
#include <sstream>
#include "Map.h"
#include "Camera.h"
#include "Controls.h"
#include "Texture.h"
#include "VoxelRenderMap.h"
#include "DeepSpaceRenderer.h"

# define M_PI           3.14159265358979323846  /* pi */

int main(int argc, char** argv) {

  Camera camera;
  Controls controls;
  RenderWindow window(1920, 1080);


  // Dark blue background
  glClearColor(0.00f, 0.00f, 0.00f, 1.0f);

  // Enable depth test
  glEnable(GL_DEPTH_TEST);
  // Accept fragment if it closer to the camera than the former one
  glDepthFunc(GL_LESS);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Cull triangles which normal is not towards the camera
  // glEnable(GL_CULL_FACE);

  // Create and compile our GLSL program from the shaders
  GLuint BlockProgramID = LoadShaders("BlockVertexShader.vert",
                                 "BlockFragmentShader.frag");

  // Get a handle for our "MVP" uniform
  GLuint MatrixID = glGetUniformLocation(BlockProgramID, "MVP");

  // Get a handle for our "myTextureSampler" uniform
  GLuint TextureID = glGetUniformLocation(BlockProgramID, "myTextureSampler");


  // Create and compile our GLSL program from the shaders
  GLuint SpaceProgramID = LoadShaders("DeepSpaceVertexShader.vert",
                                      "DeepSpaceFragmentShader.frag");

  // Get a handle for our "MVP" uniform
  GLuint SpaceMatrixID = glGetUniformLocation(SpaceProgramID, "MVP");

  // Get a handle for our "myTextureSampler" uniform
  GLuint SpaceTextureID = glGetUniformLocation(SpaceProgramID, "myTextureSampler");


  FPSCounter Counter;


  bool run = true;

  //for (int x = 0; x < 100; x += 32) {
  //  for (int z = 0; z < 100; z += 32) {
  //    Chunks.push_back(VoxelChunk({x, 0, z}));
  ///  }
  //}

  VoxelChunk Chunk({0, 0, 0});
  Chunk.generateSpaceShip();

  VoxelChunk Chunk2({160, 0, 0});
  Chunk2.generateMeteor();

  VoxelRenderMap Renderer(Chunk);
  VoxelRenderMap Renderer2(Chunk2);

  DeepSpaceRenderer DeepSpace;

  MovingLight CameraLight(&Chunk);

  Space space;
  space.add(Chunk);
  space.add(Chunk2);

  MovingEntity Player(&space);

  std::vector<Voxel::Types> BlockTypes = {
    Voxel::CRATE,
    Voxel::STEEL_FLOOR,
    Voxel::LAMP,
    Voxel::GENERATOR,
    Voxel::AIRLOCK
  };
  Voxel::Types SelectedType = BlockTypes[0];

  Uint64 LAST = 0;

  do {

    Uint64 NOW = SDL_GetPerformanceCounter();
    float deltaTime = (((float) NOW - LAST) / SDL_GetPerformanceFrequency());
    if (LAST == 0)
      deltaTime = 0;
    LAST = NOW;

    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    // Compute the MVP matrix from keyboard and mouse input
    glm::mat4 ProjectionMatrix = camera.getProjectionMatrix();
    glm::mat4 ViewMatrix = camera.getViewMatrix();
    glm::mat4 ModelMatrix = glm::mat4(1.0);
    glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

    // Use our shader
    glUseProgram(BlockProgramID);

    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

    Renderer.draw();
    Renderer2.draw();

    // Use our shader
    glUseProgram(SpaceProgramID);

    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    glUniformMatrix4fv(SpaceMatrixID, 1, GL_FALSE, &MVP[0][0]);

    DeepSpace.draw();

    Counter.addFrame();

    window.swap();

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
        run = false;
      controls.handleEvent(event);
      camera.handleEvent(event);
    }
    controls.update();
    //camera.updatePos(controls.getX(), controls.getZ());

    if (controls.jumpPoll())
      if (Player.onGround())
        Player.jump();
    Player.setMove(camera.getHorizAngle(), controls.getX(), controls.getY(), controls.getZ());
    for (float deltaTimeCopy = deltaTime - 1 / 30.0f; true; deltaTimeCopy -= 1 / 30.0f) {
      if (deltaTimeCopy > 0)
        Player.update(1 / 30.0f);
      else {
        Player.update(deltaTimeCopy + 1 / 30.0f);
        break;
      }
    }
    camera.setPos(Player.position().x, Player.position().y, Player.position().z);

    Chunk.update(deltaTime);

    // std::cout << "Current V( " << Player.position().toVoxelPos() << "): " << Chunk.get(Player.position().toVoxelPos()).getName() << std::endl;

    if (controls.getBlockType() < BlockTypes.size())
      SelectedType = BlockTypes[controls.getBlockType()];

    if (false) {
      auto cameraPos = camera.getPosition();
      v3 cameraVoxel((int64_t) cameraPos.x, (int64_t) cameraPos.y,
                     (int64_t) cameraPos.z);
      if (CameraLight.setPos(cameraVoxel)) {
        Renderer.recreateSurrounding(cameraVoxel);
      }
    }


    if (controls.leftMousePoll()) {
      for (int i = 0; i < 20; ++i) {
        auto cameraPos = camera.getPosition() + camera.getDirection(i / 4.0f);

        v3 cameraVoxel((int64_t) cameraPos.x, (int64_t) cameraPos.y,
                       (int64_t) cameraPos.z);

        if (!space.get(cameraVoxel).isBuildable())
          continue;

        space.getChunk(cameraVoxel)->setBlock(cameraVoxel, Voxel::AIR);

        Renderer.recreateSurrounding(cameraVoxel);
        Renderer2.recreateSurrounding(cameraVoxel);
        break;
      }
    }

    if (controls.rightMousePoll()) {
      vec3 lastFreePos = camera.getPosition();
      for (int i = 0; i < 20; ++i) {
        auto testPos = camera.getPosition() + camera.getDirection(i / 4.0f);

        v3 cameraVoxel((int64_t) testPos.x, (int64_t) testPos.y,
                       (int64_t) testPos.z);

        if (!space.get(cameraVoxel).isBuildable()) {
          lastFreePos = testPos;
          continue;
        }

        if (lastFreePos != camera.getPosition()) {
          v3 lastFreeVoxel((int64_t) lastFreePos.x, (int64_t) lastFreePos.y,
                         (int64_t) lastFreePos.z);
          if (auto C = space.getChunk(lastFreeVoxel)) {
            if (!Player.isCollidingWith(lastFreeVoxel)) {
              C->setBlock(lastFreeVoxel, SelectedType);
              Renderer.recreateSurrounding(lastFreeVoxel);
              Renderer2.recreateSurrounding(lastFreeVoxel);
            }
          }

        }
        break;

      }
    }

  }
  while (run);

  // Cleanup VBO and shader
  glDeleteProgram(BlockProgramID);
  glDeleteTextures(1, &TextureID);

  glDeleteProgram(SpaceProgramID);
  glDeleteTextures(1, &SpaceTextureID);

  return 0;
}

