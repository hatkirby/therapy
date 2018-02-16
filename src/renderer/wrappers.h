#ifndef WRAPPERS_H_1EE0965B
#define WRAPPERS_H_1EE0965B

#include "gl.h"
#include <utility>

class GLVertexArray {
public:

  GLVertexArray()
  {
    glGenVertexArrays(1, &id_);
  }

  GLVertexArray(const GLVertexArray& other) = delete;
  GLVertexArray& operator=(const GLVertexArray& other) = delete;

  GLVertexArray(GLVertexArray&& other) : GLVertexArray()
  {
    std::swap(id_, other.id_);
  }

  GLVertexArray& operator=(GLVertexArray&& other)
  {
    std::swap(id_, other.id_);

    return *this;
  }

  ~GLVertexArray()
  {
    glDeleteVertexArrays(1, &id_);
  }

  inline GLuint getId() const
  {
    return id_;
  }

private:

  GLuint id_;
};

class GLFramebuffer {
public:

  GLFramebuffer()
  {
    glGenFramebuffers(1, &id_);
  }

  GLFramebuffer(const GLFramebuffer& other) = delete;
  GLFramebuffer& operator=(const GLFramebuffer& other) = delete;

  GLFramebuffer(GLFramebuffer&& other) : GLFramebuffer()
  {
    std::swap(id_, other.id_);
  }

  GLFramebuffer& operator=(GLFramebuffer&& other)
  {
    std::swap(id_, other.id_);

    return *this;
  }

  ~GLFramebuffer()
  {
    glDeleteFramebuffers(1, &id_);
  }

  inline GLuint getId() const
  {
    return id_;
  }

private:

  GLuint id_;
};

class GLRenderbuffer {
public:

  GLRenderbuffer()
  {
    glGenRenderbuffers(1, &id_);
  }

  GLRenderbuffer(const GLRenderbuffer& other) = delete;
  GLRenderbuffer& operator=(const GLRenderbuffer& other) = delete;

  GLRenderbuffer(GLRenderbuffer&& other) : GLRenderbuffer()
  {
    std::swap(id_, other.id_);
  }

  GLRenderbuffer& operator=(GLRenderbuffer&& other)
  {
    std::swap(id_, other.id_);

    return *this;
  }

  ~GLRenderbuffer()
  {
    glDeleteRenderbuffers(1, &id_);
  }

  inline GLuint getId() const
  {
    return id_;
  }

private:

  GLuint id_;
};

class GLBuffer {
public:

  GLBuffer()
  {
    glGenBuffers(1, &id_);
  }

  GLBuffer(const GLBuffer& other) = delete;
  GLBuffer& operator=(const GLBuffer& other) = delete;

  GLBuffer(GLBuffer&& other) : GLBuffer()
  {
    std::swap(id_, other.id_);
  }

  GLBuffer& operator=(GLBuffer&& other)
  {
    std::swap(id_, other.id_);

    return *this;
  }

  ~GLBuffer()
  {
    glDeleteBuffers(1, &id_);
  }

  inline GLuint getId() const
  {
    return id_;
  }

private:

  GLuint id_;
};

class GLTexture {
public:

  GLTexture()
  {
    glGenTextures(1, &id_);
  }

  GLTexture(const GLTexture& other) = delete;
  GLTexture& operator=(const GLTexture& other) = delete;

  GLTexture(GLTexture&& other) : GLTexture()
  {
    std::swap(id_, other.id_);
  }

  GLTexture& operator=(GLTexture&& other)
  {
    std::swap(id_, other.id_);

    return *this;
  }

  ~GLTexture()
  {
    glDeleteTextures(1, &id_);
  }

  inline GLuint getId() const
  {
    return id_;
  }

private:

  GLuint id_;
};

class GLShader {
public:

  GLShader(GLenum type)
  {
    id_ = glCreateShader(type);
  }

  GLShader(const GLShader& other) = delete;
  GLShader& operator=(const GLShader& other) = delete;

  GLShader(GLShader&& other) : GLShader(GL_VERTEX_SHADER)
  {
    std::swap(id_, other.id_);
  }

  GLShader& operator=(GLShader&& other)
  {
    std::swap(id_, other.id_);

    return *this;
  }

  ~GLShader()
  {
    glDeleteShader(id_);
  }

  inline GLuint getId() const
  {
    return id_;
  }

private:

  GLuint id_;
};

class GLProgram {
public:

  GLProgram()
  {
    id_ = glCreateProgram();
  }

  GLProgram(const GLProgram& other) = delete;
  GLProgram& operator=(const GLProgram& other) = delete;

  GLProgram(GLProgram&& other) : GLProgram()
  {
    std::swap(id_, other.id_);
  }

  GLProgram& operator=(GLProgram&& other)
  {
    std::swap(id_, other.id_);

    return *this;
  }

  ~GLProgram()
  {
    glDeleteProgram(id_);
  }

  inline GLuint getId() const
  {
    return id_;
  }

private:

  GLuint id_;
};

#endif /* end of include guard: WRAPPERS_H_1EE0965B */
