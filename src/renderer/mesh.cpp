#include "mesh.h"
#include <fstream>
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include "util.h"

Mesh::Mesh(std::string filename)
{
  std::ifstream meshfile(filename);
  if (!meshfile.is_open())
  {
    throw std::invalid_argument("Could not open mesh file");
  }

  std::vector<glm::vec3> tempVertices;
  std::vector<glm::vec2> tempUvs;
  std::vector<glm::vec3> tempNormals;

  std::vector<glm::vec3> outVertices;
  std::vector<glm::vec2> outUvs;
  std::vector<glm::vec3> outNormals;
  std::map<element, unsigned short> elementIds;
  std::vector<unsigned short> indices;

  while (meshfile)
  {
    std::string linetype;
    meshfile >> linetype;

    if (linetype == "v")
    {
      glm::vec3 vertex;
      meshfile >> vertex.x >> vertex.y >> vertex.z;

      tempVertices.push_back(std::move(vertex));
    } else if (linetype == "vt")
    {
      glm::vec2 uv;
      meshfile >> uv.x >> uv.y;

      tempUvs.push_back(std::move(uv));
    } else if (linetype == "vn")
    {
      glm::vec3 normal;
      meshfile >> normal.x >> normal.y >> normal.z;

      tempNormals.push_back(std::move(normal));
    } else if (linetype == "f")
    {
      element elements[3];

      meshfile
        >> elements[0].vertexId >> chlit('/')
        >> elements[0].uvId >> chlit('/')
        >> elements[0].normalId
        >> elements[1].vertexId >> chlit('/')
        >> elements[1].uvId >> chlit('/')
        >> elements[1].normalId
        >> elements[2].vertexId >> chlit('/')
        >> elements[2].uvId >> chlit('/')
        >> elements[2].normalId;

      for (size_t i = 0; i < 3; i++)
      {
        if (!elementIds.count(elements[i]))
        {
          elementIds[elements[i]] = outVertices.size();

          outVertices.push_back(tempVertices[elements[i].vertexId - 1]);
          outUvs.push_back(tempUvs[elements[i].uvId - 1]);
          outNormals.push_back(tempNormals[elements[i].normalId - 1]);
        }

        indices.push_back(elementIds[elements[i]]);
      }
    }
  }

  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_.getId());
  glBufferData(
    GL_ARRAY_BUFFER,
    outVertices.size() * sizeof(glm::vec3),
    outVertices.data(),
    GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, uvBuffer_.getId());
  glBufferData(
    GL_ARRAY_BUFFER,
    outUvs.size() * sizeof(glm::vec2),
    outUvs.data(),
    GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, normalBuffer_.getId());
  glBufferData(
    GL_ARRAY_BUFFER,
    outNormals.size() * sizeof(glm::vec3),
    outNormals.data(),
    GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer_.getId());
  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER,
    indices.size() * sizeof(unsigned short),
    indices.data(),
    GL_STATIC_DRAW);

  indexCount_ = indices.size();
}
