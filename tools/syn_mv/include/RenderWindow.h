#ifndef RENDER_WINDOW_H
#define RENDER_WINDOW_H

#include <GLFW/glfw3.h>

class RenderWindow {
 public:
  ////////////////
  // Constructor //
  ////////////////
  RenderWindow(int width, int height, const char *title, int samples = 4,
               int version_major = 3, int version_minor = 3);
  RenderWindow(const RenderWindow &wnd);

  //////////////
  // Operators //
  //////////////
  RenderWindow &operator=(const RenderWindow &wnd);

  /////////////////
  // Other Methods//
  /////////////////
  int getWidth() const;
  int getHeight() const;
  void resize(int width, int height);

  void makeContextCurrent();

  bool isOpen() const;
  bool pollEvents() const;
  bool clear(float r, float g, float b, float a) const;
  bool swapBuffers() const;
  bool close() const;

 private:
  void _assign(const RenderWindow &wnd);

  int _width, _height;
  int _samples;
  int _version_major, _version_minor;
  GLFWwindow *_wnd;
};

#endif
