#include "shader.h"
#include <fstream>
#include <vector>
#include "util.h"

Shader::Shader(std::string name)
{
  GLShader vertexShader(GL_VERTEX_SHADER);
  GLShader fragmentShader(GL_FRAGMENT_SHADER);

  std::ifstream vertexFile("shaders/" + name + ".vertex");
  std::ifstream fragmentFile("shaders/" + name + ".fragment");

  std::string vertexCode(slurp(vertexFile));
  std::string fragmentCode(slurp(fragmentFile));

  const char* vertexCodePtr = vertexCode.c_str();
  const char* fragmentCodePtr = fragmentCode.c_str();

  glShaderSource(vertexShader.getId(), 1, &vertexCodePtr, nullptr);
  glShaderSource(fragmentShader.getId(), 1, &fragmentCodePtr, nullptr);

  glCompileShader(vertexShader.getId());
  glCompileShader(fragmentShader.getId());

#ifdef DEBUG
  GLint result = GL_FALSE;
  int infoLogLength;

  glGetShaderiv(vertexShader.getId(), GL_COMPILE_STATUS, &result);

  if (result == GL_FALSE)
  {
    glGetShaderiv(vertexShader.getId(), GL_INFO_LOG_LENGTH, &infoLogLength);

    std::vector<char> errMsg(infoLogLength);
    glGetShaderInfoLog(
      vertexShader.getId(),
      infoLogLength,
      nullptr,
      errMsg.data());

    throw std::gl_error("Could not compile shader", errMsg.data());
  }

  glGetShaderiv(fragmentShader.getId(), GL_COMPILE_STATUS, &result);

  if (result == GL_FALSE)
  {
    glGetShaderiv(fragmentShader.getId(), GL_INFO_LOG_LENGTH, &infoLogLength);

    std::vector<char> errMsg(infoLogLength);
    glGetShaderInfoLog(
      fragmentShader.getId(),
      infoLogLength,
      nullptr,
      errMsg.data());

    throw std::gl_error("Could not compile shader", errMsg.data());
  }
#endif

  glAttachShader(program_.getId(), vertexShader.getId());
  glAttachShader(program_.getId(), fragmentShader.getId());
  glLinkProgram(program_.getId());

#ifdef DEBUG
  glGetProgramiv(program_.getId(), GL_LINK_STATUS, &result);

  if (result == GL_FALSE)
  {
    glGetProgramiv(program_.getId(), GL_INFO_LOG_LENGTH, &infoLogLength);

    std::vector<char> errMsg(infoLogLength);
    glGetProgramInfoLog(
      program_.getId(),
      infoLogLength,
      nullptr,
      errMsg.data());

    throw std::gl_error("Could not link shader program", errMsg.data());
  }
#endif
}
