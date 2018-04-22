#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <GL/glew.h>

#include "io.hpp"

GLuint compileShaders(std::string vertFile, std::string fragFile)
{
	GLuint programID;
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	std::string vertexShaderCode = readFile(vertFile);
  const char* vertAdapter = vertexShaderCode.data();
  glShaderSource(vertexShaderID, 1, &vertAdapter, 0);
  glCompileShader(vertexShaderID);

  std::string fragmentShaderCode = readFile(fragFile);
  const char* fragAdapter = fragmentShaderCode.data();
  glShaderSource(fragmentShaderID, 1, &fragAdapter, 0);
  glCompileShader(fragmentShaderID);

  programID = glCreateProgram();
  glAttachShader(programID, vertexShaderID);
  glAttachShader(programID, fragmentShaderID);
  glLinkProgram(programID);

  GLint success = 0;
  glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &success);
  if(success == GL_FALSE)
  {
    GLint maxLength = 0;
	  glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &maxLength);
    std::vector<GLchar> errorLog(maxLength);

    glGetShaderInfoLog(fragmentShaderID, maxLength, &maxLength, &errorLog[0]);
    std::cout << errorLog.data() << std::endl;

    glDeleteShader(fragmentShaderID);
    throw std::runtime_error("Failed to compile fragment shader.\n");
  }
	glDeleteShader(fragmentShaderID);
  glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &success);
  if(success == GL_FALSE)
  {
    GLint maxLength = 0;
	  glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &maxLength);

    std::vector<GLchar> errorLog(maxLength);
    glGetShaderInfoLog(vertexShaderID, maxLength, &maxLength, &errorLog[0]);
    std::cout << errorLog.data() << std::endl;

    glDeleteShader(vertexShaderID);
    throw std::runtime_error("Failed to compile vertex shader.\n");
  }
	glDeleteShader(vertexShaderID);
	return programID;
}
