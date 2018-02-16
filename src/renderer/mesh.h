#ifndef MESH_H_76B72E12
#define MESH_H_76B72E12

#include <string>
#include "gl.h"
#include "wrappers.h"

class Mesh {
public:

  Mesh(std::string filename);

  Mesh(const Mesh& other) = delete;
  Mesh& operator=(const Mesh& other) = delete;

  inline GLuint getVertexBufferId() const
  {
    return vertexBuffer_.getId();
  }

  inline GLuint getUvBufferId() const
  {
    return uvBuffer_.getId();
  }

  inline GLuint getNormalBufferId() const
  {
    return normalBuffer_.getId();
  }

  inline GLuint getIndexBufferId() const
  {
    return indexBuffer_.getId();
  }

  inline size_t getIndexCount() const
  {
    return indexCount_;
  }

private:

  struct element {
    size_t vertexId;
    size_t uvId;
    size_t normalId;

    bool operator<(const element& other) const
    {
      return std::tie(vertexId, uvId, normalId) <
        std::tie(other.vertexId, other.uvId, other.normalId);
    }
  };

  GLBuffer vertexBuffer_;
  GLBuffer uvBuffer_;
  GLBuffer normalBuffer_;
  GLBuffer indexBuffer_;
  size_t indexCount_;
};

#endif /* end of include guard: MESH_H_76B72E12 */
