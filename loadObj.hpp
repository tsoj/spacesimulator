#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <glm/glm.hpp>

#include "vertex.hpp"
#include "io.hpp"

struct Obj
{
  struct Mtl
  {
    glm::vec3 ambientColor;
    glm::vec3 diffuseColor;
    glm::vec3 specularColor;
    glm::float_t transparency;
    glm::float_t shininess;

    std::string ambientTexture;
    std::string diffuseTexture;
    std::string specularColorTexture;
    std::string specularHighlightTexture;
    std::string alphaTexture;
    std::string normalMap;

    static void loadMtl(std::string filePath, std::vector<Mtl>& materials, std::map<std::string, size_t>& mtlIndices)
    {
      std::string mtlData = readFile(filePath);

      std::stringstream modelDataStream(mtlData);
    	std::string bufferString;
    	while (getline(modelDataStream, bufferString))
    	{
    		std::stringstream bufferStringStream(bufferString);
    		bufferStringStream >> bufferString;

        if(bufferString == "newmtl")
        {
          bufferStringStream >> bufferString;
          mtlIndices[bufferString] = materials.size();
          materials.emplace_back();
        }
        else if(bufferString == "Ka")
        {
          bufferStringStream >> bufferString;
          materials.back().ambientColor.r = std::atof(bufferString.c_str());
          bufferStringStream >> bufferString;
          materials.back().ambientColor.g = std::atof(bufferString.c_str());
          bufferStringStream >> bufferString;
          materials.back().ambientColor.b = std::atof(bufferString.c_str());
        }
        else if(bufferString == "Kd")
        {
          bufferStringStream >> bufferString;
          materials.back().diffuseColor.r = std::atof(bufferString.c_str());
          bufferStringStream >> bufferString;
          materials.back().diffuseColor.g = std::atof(bufferString.c_str());
          bufferStringStream >> bufferString;
          materials.back().diffuseColor.b = std::atof(bufferString.c_str());
        }
        else if(bufferString == "Ks")
        {
          bufferStringStream >> bufferString;
          materials.back().specularColor.r = std::atof(bufferString.c_str());
          bufferStringStream >> bufferString;
          materials.back().specularColor.g = std::atof(bufferString.c_str());
          bufferStringStream >> bufferString;
          materials.back().specularColor.b = std::atof(bufferString.c_str());
        }
        else if(bufferString == "d" || bufferString == "Tr")
        {
          bufferStringStream >> bufferString;
          materials.back().transparency = std::atof(bufferString.c_str());
        }
        else if(bufferString == "Ns")
        {
          bufferStringStream >> bufferString;
          materials.back().shininess = std::atof(bufferString.c_str());
        }
        else if(bufferString == "map_Ka")
        {
          bufferStringStream >> bufferString;
          materials.back().ambientTexture = bufferString;
        }
        else if(bufferString == "map_Kd")
        {
          bufferStringStream >> bufferString;
          materials.back().diffuseTexture = bufferString;
        }
        else if(bufferString == "map_Ks")
        {
          bufferStringStream >> bufferString;
          materials.back().specularColorTexture = bufferString;
        }
        else if(bufferString == "map_Ns")
        {
          bufferStringStream >> bufferString;
          materials.back().specularHighlightTexture = bufferString;
        }
        else if(bufferString == "map_d")
        {
          bufferStringStream >> bufferString;
          materials.back().alphaTexture = bufferString;
        }
        else if(bufferString == "bump" || bufferString == "map_bump")
        {
          bufferStringStream >> bufferString;
          materials.back().normalMap = bufferString;
        }
      }
    }
  };
  struct O
  {
    std::vector<Vertex> vertices = std::vector<Vertex>();
    Mtl mtl;
  };

  std::vector<O> objects = std::vector<O>();

  static Obj loadObj(std::string filePath)
  {
    std::string modelData = readFile(filePath);

    std::vector<glm::vec3> positions = std::vector<glm::vec3>();
    std::vector<glm::vec2> textureCoordinates = std::vector<glm::vec2>();
    std::vector<glm::vec3> normals = std::vector<glm::vec3>();

    std::vector<Vertex> faces = std::vector<Vertex>();

    std::vector<Mtl> mtls = std::vector<Mtl>();
    std::map<std::string, size_t> mtlIndices = std::map<std::string, size_t>();

    Obj ret = Obj();

    std::stringstream modelDataStream(modelData);
  	std::string bufferString;
  	while (getline(modelDataStream, bufferString))
  	{
  		std::stringstream bufferStringStream(bufferString);
  		bufferStringStream >> bufferString;
  		if (bufferString == "o")
  		{
        ret.objects.push_back(O());
  		}
  		else if (bufferString == "mtllib")
  		{
  		  bufferStringStream >> bufferString;
        Mtl::loadMtl(bufferString, mtls, mtlIndices);
  		}
  		else if (bufferString == "usemtl")
  		{
  		  bufferStringStream >> bufferString;
        if(mtlIndices.find(bufferString) != mtlIndices.end())
        {
          ret.objects.back().mtl = mtls[mtlIndices[bufferString]];
        }
  		}
  		else if (bufferString == "v")
  		{
    		glm::vec3 tempVec;
    		bufferStringStream >> bufferString;
    		tempVec.x = std::stof(bufferString);
    		bufferStringStream >> bufferString;
    		tempVec.y = std::stof(bufferString);
    		bufferStringStream >> bufferString;
    		tempVec.z = std::stof(bufferString);
    		positions.push_back(tempVec);
  		}
  		else if (bufferString == "vt")
  		{
  			glm::vec2 tempVec;
  			bufferStringStream >> bufferString;
  			tempVec.x = std::stof(bufferString);
  			bufferStringStream >> bufferString;
  			tempVec.y = 1.0f - std::stof(bufferString);
  			textureCoordinates.push_back(tempVec);
  		}
  		else if (bufferString == "vn")
  		{
  			glm::vec3 tempVec;
  			bufferStringStream >> bufferString;
  			tempVec.x = std::stof(bufferString);
  			bufferStringStream >> bufferString;
  			tempVec.y = std::stof(bufferString);
  			bufferStringStream >> bufferString;
  			tempVec.z = std::stof(bufferString);
  			normals.push_back(tempVec);
  		}
  		else if (bufferString == "f")
  		{
        while (bufferStringStream >> bufferString)
        {
          Vertex tempVertex;
          std::stringstream tempStream(bufferString);
          std::string tempBufferString;
          int iterator = 0;
          while (std::getline(tempStream, tempBufferString, '/'))
          {
            if (tempBufferString == "")
            {
              if (iterator == 0)
              {
                tempVertex.position = glm::vec3(0.0, 0.0, 0.0);
              }
              else if (iterator == 1)
              {
                tempVertex.textureCoordinate = glm::vec2(-1.0, -1.0);
              }
              else if (iterator == 2)
              {
                tempVertex.normal = glm::vec3(0.0, 1.0, 0.0);
              }
            }
            else if (iterator == 0)
            {
              tempVertex.position = positions[stoi(tempBufferString) - 1];
            }
            else if (iterator == 1)
            {
              tempVertex.textureCoordinate = textureCoordinates[stoi(tempBufferString) - 1];
            }
            else if (iterator == 2)
            {
              tempVertex.normal = normals[stoi(tempBufferString) - 1];
            }
            iterator++;
          }
          ret.objects.back().vertices.push_back(tempVertex);
        }

        auto size = ret.objects.back().vertices.size();
        glm::vec3 v0 = ret.objects.back().vertices[size-1].position;
        glm::vec3 v1 = ret.objects.back().vertices[size-2].position;
        glm::vec3 v2 = ret.objects.back().vertices[size-3].position;

        glm::vec3 dv0 = v1-v0;
        glm::vec3 dv1 = v2-v0;

        if(ret.objects.back().vertices[size-1].textureCoordinate == glm::vec2(-1.0, -1.0))
        {
          ret.objects.back().vertices[size-1].textureCoordinate = glm::vec2(1.0, 1.0);
          ret.objects.back().vertices[size-2].textureCoordinate = glm::vec2(0.0, 1.0);
          ret.objects.back().vertices[size-3].textureCoordinate = glm::vec2(0.0, 0.0);
        }

        glm::vec2 t0 = ret.objects.back().vertices[size-1].textureCoordinate;
        glm::vec2 t1 = ret.objects.back().vertices[size-2].textureCoordinate;
        glm::vec2 t2 = ret.objects.back().vertices[size-3].textureCoordinate;

        glm::vec2 dt0 = t1-t0;
        glm::vec2 dt1 = t2-t0;

        glm::vec3 tmpTangent = (dv0*dt1.y - dv1*dt0.y) / (dt0.x*dt1.y - dt0.y*dt1.x);

        for(size_t i = 1; i<=3; i++)
        {
          glm::vec3 biTangent = glm::cross(tmpTangent, ret.objects.back().vertices[size-i].normal);
          glm::vec3 realTangent = glm::cross(ret.objects.back().vertices[size-i].normal, biTangent);
          ret.objects.back().vertices[size-i].tangent = realTangent;
        }
      }
  	}
    return ret;
  }
};
