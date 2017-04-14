#ifndef FPSCOUNTER_H
#define FPSCOUNTER_H

#include <chrono>
#include <iostream>

class FPSCounter {
  unsigned frames = 0;
  std::chrono::steady_clock::time_point lastStart;

public:
  FPSCounter() {
    lastStart = std::chrono::steady_clock::now();
  }

  long getMillis() {
    auto diff = std::chrono::steady_clock::now() - lastStart;
    return std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
  }

  void addFrame() {
    ++frames;
    auto millis = getMillis();
    if (millis >= 1000) {
      lastStart = std::chrono::steady_clock::now();
      std::cout << (frames / (millis / 1000.0)) << std::endl;
      frames = 0;
    }
  }
};

#endif // FPSCOUNTER_H
