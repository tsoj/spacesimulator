#pragma once
#include <string>
#include <fstream>

std::string readFile(std::string filePath)
{
  std::string ret = "";
  std::string tmp;
  std::ifstream file;
  file.open(filePath);
  if(!file.is_open())
  {
  	throw std::runtime_error("Failed to open file: " + filePath);
  }
  while (std::getline(file, tmp))
  {
    ret += tmp + "\n";
  }
  file.close();
  return ret;
}
