#pragma once

#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace Window
{
  inline GLFWwindow* window;
  inline void init()
  {
    static std::vector<const char*> requiredExtensions =
    {
      "GL_ARB_vertex_attrib_64bit",
      "GL_ARB_gpu_shader_fp64"
    };

    static auto errorCallback_GLFW = [](int error, const char* description)
    {
      throw std::runtime_error("Error: " + std::string(description) + " (" + std::to_string(error) + ")\n");
    };
    glfwSetErrorCallback(errorCallback_GLFW);
    if (!glfwInit())
    {
      throw std::runtime_error("Failed to initialize GLFW.\n");
    }

  	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

  	window = glfwCreateWindow(800, 600, "OpenglTest", NULL, NULL);
    if (!window)
    {
      glfwTerminate();
      throw std::runtime_error("Failed to initialize Window or context.\n");
    }
  	glfwMakeContextCurrent(window);

  	GLenum err = glewInit();
    if (GLEW_OK != err)
    {
      throw std::runtime_error("Failed to initialize GLEW: "+ std::string((char*)glewGetErrorString(err)) + "\n");
    }
    if(not [](){
      for(auto c : requiredExtensions){ if(!(glewIsSupported(c))){return false;} } return true;
    })
    {
      throw std::runtime_error("Failed to find required extensions.\n");
    }
  	glfwSwapInterval(1);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glEnable(GL_DEPTH_TEST);
  	glEnable(GL_CULL_FACE);
  	//glFrontFace(GL_CW);

  	glClearColor(0.1f, 0.2f, 0.4f, 1.0f);
  }
}
