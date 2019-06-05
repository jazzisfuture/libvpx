#include <iostream>
#include "RenderWindow.h"

RenderWindow::RenderWindow(int width, int height, const char *title,
                           int samples, int version_major, int version_minor) {
  _width = width;
  _height = height;
  _samples = samples;
  _version_major = version_major;
  _version_minor = version_minor;

  // initalize glfw
  if (!glfwInit()) {
    std::cout << "Failed to initialize GLFW" << std::endl;
    exit(1);
  }

  // configure window hints
  glfwWindowHint(GLFW_SAMPLES, _samples);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, _version_major);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, _version_minor);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  _wnd = glfwCreateWindow(_width, _height, title, NULL, NULL);
  if (!_wnd) {
    std::cout << "Failed to open GLFW window." << std::endl;
    this->close();
    exit(1);
  }

  glfwSetInputMode(_wnd, GLFW_STICKY_KEYS, GL_TRUE);
}

RenderWindow::RenderWindow(const RenderWindow &wnd) { this->_assign(wnd); }

RenderWindow &RenderWindow::operator=(const RenderWindow &wnd) {
  this->_assign(wnd);
}

int RenderWindow::getWidth() const { return _width; }
int RenderWindow::getHeight() const { return _height; }

void RenderWindow::resize(int width, int height) {
  _width = width;
  _height = height;
  glfwSetWindowSize(_wnd, _width, _height);
}

void RenderWindow::makeContextCurrent() { glfwMakeContextCurrent(_wnd); }

bool RenderWindow::isOpen() const {
  return glfwGetKey(_wnd, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
         glfwWindowShouldClose(_wnd) == 0;
}

bool RenderWindow::pollEvents() const { glfwPollEvents(); }

bool RenderWindow::clear(float r, float g, float b, float a) const {
  glClearColor(r, g, b, a);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

bool RenderWindow::swapBuffers() const { glfwSwapBuffers(_wnd); }

bool RenderWindow::close() const { glfwTerminate(); }

void RenderWindow::_assign(const RenderWindow &wnd) {
  _width = wnd._width;
  _height = wnd._height;
  _samples = wnd._samples;
  _version_major = wnd._version_major;
  _version_minor = wnd._version_minor;
  _wnd = wnd._wnd;
}
