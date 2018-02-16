#ifndef SHADER_H_25115B63
#define SHADER_H_25115B63

#include <string>
#include <stdexcept>
#include "gl.h"
#include "wrappers.h"

class gl_error : public std::logic_error {
public:

  gl_error(
    const char* msg,
    std::string info) :
      std::logic_error(msg),
      info_(std::move(info))
  {
  }

  gl_error(
    std::string& msg,
    std::string info) :
      std::logic_error(msg),
      info_(std::move(info))
  {
  }

  inline const std::string& getInfo() const
  {
    return info_;
  }

private:

  std::string info_;
};

class Shader {
public:

  Shader(std::string name);

  inline void use()
  {
    glUseProgram(program_.getId());
  }

  inline GLint getUniformLocation(const GLchar* name)
  {
    return glGetUniformLocation(program_.getId(), name);
  }

private:

  GLProgram program_;
};

#endif /* end of include guard: SHADER_H_25115B63 */
