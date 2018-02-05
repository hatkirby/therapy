#include "renderer.h"
#include <string>
#include <fstream>
#include <vector>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "consts.h"

// include stb_image
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#include "stb_image.h"

static bool rendererInitialized = false;

static GLFWwindow* window;

static GLuint generic_framebuffer; // The framebuffer
static GLuint bloom_framebuffer;
static GLuint bloom_depthbuffer;
static int buffer_width = 1024;
static int buffer_height = 768;

static GLuint ntscShader; // The NTSC shader
static GLuint finalShader; // The passthrough shader
static GLuint blitShader; // The blitting shader
static GLuint fillShader; // The fill shader
static GLuint bloom1Shader;
static GLuint bloom2Shader;

// The buffers for the NTSC rendering process
static GLuint renderedTex1;
static GLuint renderedTex2;
static GLuint renderedTexBufs[2];
static int curBuf;
static GLuint artifactsTex;
static GLuint scanlinesTex;
static GLuint preBloomTex;
static GLuint bloomPassTex1;
static GLuint bloomPassTex2;

// The VAO
static GLuint VertexArrayID;

// A plane that fills the renderbuffer
static GLuint quad_vertexbuffer;

// Buffers for the mesh
static GLuint mesh_vertexbuffer;
static GLuint mesh_uvbuffer;
static GLuint mesh_normalbuffer;
static int mesh_numvertices;

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path)
{
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open())
    {
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , nullptr);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> VertexShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, nullptr, &VertexShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , nullptr);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, nullptr, &FragmentShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

    // Link the program
    fprintf(stdout, "Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> ProgramErrorMessage( glm::max(InfoLogLength, int(1)) );
    glGetProgramInfoLog(ProgramID, InfoLogLength, nullptr, &ProgramErrorMessage[0]);
    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

void flipImageData(unsigned char* data, int width, int height, int comps)
{
  unsigned char* data_copy = (unsigned char*) malloc(width*height*comps*sizeof(unsigned char));
  memcpy(data_copy, data, width*height*comps);

  int row_size = width * comps;

  for (int i=0;i<height;i++)
  {
    memcpy(data + (row_size*i), data_copy + (row_size*(height-i-1)), row_size);
  }

  free(data_copy);
}

void loadMesh(const char* filename, std::vector<glm::vec3>& out_vertices, std::vector<glm::vec2>& out_uvs, std::vector<glm::vec3>& out_normals)
{
  FILE* file = fopen(filename, "r");
  if (file == nullptr)
  {
    fprintf(stderr, "Could not open mesh file %s\n", filename);
    exit(1);
  }

  std::vector<glm::vec3> temp_vertices;
  std::vector<glm::vec2> temp_uvs;
  std::vector<glm::vec3> temp_normals;

  for (;;)
  {
    char lineHeader[256];
    int res = fscanf(file, "%s", lineHeader);
    if (res == EOF)
    {
      break;
    }

    if (!strncmp(lineHeader, "v", 2))
    {
      glm::vec3 vertex;
      fscanf(file, "%f %f %f\n", &vertex.x,&vertex.y,&vertex.z);
      temp_vertices.push_back(vertex);
    } else if (!strncmp(lineHeader, "vt", 3))
    {
      glm::vec2 uv;
      fscanf(file, "%f %f\n", &uv.x, &uv.y);
      temp_uvs.push_back(uv);
    } else if (!strncmp(lineHeader, "vn", 3))
    {
      glm::vec3 normal;
      fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
      temp_normals.push_back(normal);
    } else if (!strncmp(lineHeader, "f", 2))
    {
      int vertexIDs[3], uvIDs[3], normalIDs[3];
      fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIDs[0], &uvIDs[0], &normalIDs[0], &vertexIDs[1], &uvIDs[1], &normalIDs[1], &vertexIDs[2], &uvIDs[2], &normalIDs[2]);

      for (int i=0; i<3; i++)
      {
        out_vertices.push_back(temp_vertices[vertexIDs[i] - 1]);
        out_uvs.push_back(temp_uvs[uvIDs[i] - 1]);
        out_normals.push_back(temp_normals[normalIDs[i] - 1]);
      }
    }
  }
}

void setFramebufferSize(GLFWwindow* w, int width, int height)
{
  buffer_width = width;
  buffer_height = height;

  glDeleteFramebuffers(1, &bloom_framebuffer);
  glDeleteRenderbuffers(1, &bloom_depthbuffer);

  glGenFramebuffers(1, &bloom_framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, bloom_framebuffer);
  GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT1};
  glDrawBuffers(1, DrawBuffers);

  glGenRenderbuffers(1, &bloom_depthbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, bloom_depthbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, bloom_depthbuffer);

  glDeleteTextures(1, &preBloomTex);
  glDeleteTextures(1, &bloomPassTex1);
  glDeleteTextures(1, &bloomPassTex2);

  glGenTextures(1, &preBloomTex);
  glBindTexture(GL_TEXTURE_2D, preBloomTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glGenTextures(1, &bloomPassTex1);
  glBindTexture(GL_TEXTURE_2D, bloomPassTex1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width/4, height/4, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glGenTextures(1, &bloomPassTex2);
  glBindTexture(GL_TEXTURE_2D, bloomPassTex2);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width/4, height/4, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

GLFWwindow* initRenderer()
{
  if (rendererInitialized)
  {
    fprintf(stderr, "Renderer already initialized\n");
    exit(-1);
  }

  // Initialize GLFW
  if (!glfwInit())
  {
    fprintf(stderr, "Failed to initialize GLFW\n");
    exit(-1);
  }

  glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want version 3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Mac requires this
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Create a window
  window = glfwCreateWindow(1024, 768, "Aromatherapy", nullptr, nullptr);
  if (window == nullptr)
  {
    fprintf(stderr, "Failed to open GLFW window\n");
    glfwTerminate();
    exit(-1);
  }

  glfwMakeContextCurrent(window);
  glewExperimental = true; // Needed in core profile
  if (glewInit() != GLEW_OK)
  {
    fprintf(stderr, "Failed to initialize GLEW\n");
    exit(-1);
  }

  glfwSetFramebufferSizeCallback(window, &setFramebufferSize);

  // Set up vertex array object
  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);

  // Enable depth testing
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  // Enable blending
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Set up the framebuffer
  glGenFramebuffers(1, &generic_framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, generic_framebuffer);
  GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, DrawBuffers);

  glGenFramebuffers(1, &bloom_framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, bloom_framebuffer);
  GLenum DrawBuffers2[1] = {GL_COLOR_ATTACHMENT1};
  glDrawBuffers(1, DrawBuffers2);

  glGenRenderbuffers(1, &bloom_depthbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, bloom_depthbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1024, 768);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, bloom_depthbuffer);

  // Set up the NTSC rendering buffers
  glGenTextures(1, &renderedTex1);
  glBindTexture(GL_TEXTURE_2D, renderedTex1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, GAME_WIDTH, GAME_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  renderedTexBufs[0] = renderedTex1;

  glGenTextures(1, &renderedTex2);
  glBindTexture(GL_TEXTURE_2D, renderedTex2);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, GAME_WIDTH, GAME_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  renderedTexBufs[1] = renderedTex2;

  // Set up bloom rendering buffers
  glGenTextures(1, &preBloomTex);
  glBindTexture(GL_TEXTURE_2D, preBloomTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 768, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glGenTextures(1, &bloomPassTex1);
  glBindTexture(GL_TEXTURE_2D, bloomPassTex1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024/4, 768/4, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glGenTextures(1, &bloomPassTex2);
  glBindTexture(GL_TEXTURE_2D, bloomPassTex2);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024/4, 768/4, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  curBuf = 0;

  // Load the mesh!
  std::vector<glm::vec3> mesh_vertices;
  std::vector<glm::vec2> mesh_uvs;
  std::vector<glm::vec3> mesh_normals;
  loadMesh("res/monitor-fef.obj", mesh_vertices, mesh_uvs, mesh_normals);

  mesh_numvertices = mesh_vertices.size();

  glGenBuffers(1, &mesh_vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, mesh_vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, mesh_vertices.size() * sizeof(glm::vec3), &mesh_vertices[0], GL_STATIC_DRAW);

  glGenBuffers(1, &mesh_uvbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, mesh_uvbuffer);
  glBufferData(GL_ARRAY_BUFFER, mesh_uvs.size() * sizeof(glm::vec3), &mesh_uvs[0], GL_STATIC_DRAW);

  glGenBuffers(1, &mesh_normalbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, mesh_normalbuffer);
  glBufferData(GL_ARRAY_BUFFER, mesh_normals.size() * sizeof(glm::vec3), &mesh_normals[0], GL_STATIC_DRAW);

  // Load the vertices of a flat surface
  GLfloat g_quad_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
	};

  glGenBuffers(1, &quad_vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

  glGenTextures(1, &artifactsTex);
  glBindTexture(GL_TEXTURE_2D, artifactsTex);
  int atdw, atdh;
  unsigned char* artifactsTex_data = stbi_load("res/artifacts.bmp", &atdw, &atdh, 0, 3);
  flipImageData(artifactsTex_data, atdw, atdh, 3);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, atdw, atdh, 0, GL_RGB, GL_UNSIGNED_BYTE, artifactsTex_data);
  stbi_image_free(artifactsTex_data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
  glGenerateMipmap(GL_TEXTURE_2D);

  glGenTextures(1, &scanlinesTex);
  glBindTexture(GL_TEXTURE_2D, scanlinesTex);
  int stdw, stdh;
  unsigned char* scanlinesTex_data = stbi_load("res/scanlines_333.bmp", &stdw, &stdh, 0, 3);
  flipImageData(scanlinesTex_data, stdw, stdh, 3);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, atdw, atdh, 0, GL_RGB, GL_UNSIGNED_BYTE, scanlinesTex_data);
  stbi_image_free(scanlinesTex_data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
  glGenerateMipmap(GL_TEXTURE_2D);

  // Load the shaders
  ntscShader = LoadShaders("shaders/ntsc.vertex", "shaders/ntsc.fragment");
  finalShader = LoadShaders("shaders/final.vertex", "shaders/final.fragment");
  blitShader = LoadShaders("shaders/blit.vertex", "shaders/blit.fragment");
  fillShader = LoadShaders("shaders/fill.vertex", "shaders/fill.fragment");
  bloom1Shader = LoadShaders("shaders/bloom1.vertex", "shaders/bloom1.fragment");
  bloom2Shader = LoadShaders("shaders/bloom2.vertex", "shaders/bloom2.fragment");

  rendererInitialized = true;

  return window;
}

void destroyRenderer()
{
  if (!rendererInitialized)
  {
    fprintf(stderr, "Renderer not initialized\n");
    exit(-1);
  }

  // Delete the plane buffer
  glDeleteBuffers(1, &quad_vertexbuffer);
  glDeleteBuffers(1, &mesh_vertexbuffer);
  glDeleteBuffers(1, &mesh_uvbuffer);
  glDeleteBuffers(1, &mesh_normalbuffer);

  // Delete the shaders
  glDeleteProgram(ntscShader);
  glDeleteProgram(finalShader);
  glDeleteProgram(blitShader);
  glDeleteProgram(fillShader);
  glDeleteProgram(bloom1Shader);
  glDeleteProgram(bloom2Shader);

  // Delete the NTSC rendering buffers
  glDeleteTextures(1, &renderedTex1);
  glDeleteTextures(1, &renderedTex2);
  glDeleteTextures(1, &artifactsTex);
  glDeleteTextures(1, &scanlinesTex);
  glDeleteTextures(1, &preBloomTex);
  glDeleteTextures(1, &bloomPassTex1);
  glDeleteTextures(1, &bloomPassTex2);

  // Delete the framebuffer
  glDeleteRenderbuffers(1, &bloom_depthbuffer);
  glDeleteFramebuffers(1, &bloom_framebuffer);
  glDeleteFramebuffers(1, &generic_framebuffer);

  // Delete the VAO
  glDeleteVertexArrays(1, &VertexArrayID);

  // Kill the window
  glfwTerminate();

  rendererInitialized = false;
}

Texture::Texture(int width, int height)
{
  if (!rendererInitialized)
  {
    fprintf(stderr, "Renderer not initialized\n");
    exit(-1);
  }

  this->width = width;
  this->height = height;

  glGenTextures(1, &texID);
  glBindTexture(GL_TEXTURE_2D, texID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
}

Texture::Texture(const char* filename)
{
  if (!rendererInitialized)
  {
    fprintf(stderr, "Renderer not initialized\n");
    exit(-1);
  }

  glGenTextures(1, &texID);
  glBindTexture(GL_TEXTURE_2D, texID);
  unsigned char* data = stbi_load(filename, &width, &height, 0, 4);
  flipImageData(data, width, height, 4);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  stbi_image_free(data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

Texture::Texture(const Texture& tex)
{
  if (!rendererInitialized)
  {
    fprintf(stderr, "Renderer not initialized\n");
    exit(-1);
  }

  width = tex.width;
  height = tex.height;

  unsigned char* data = (unsigned char*) malloc(4 * width * height);
  glBindTexture(GL_TEXTURE_2D, tex.texID);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

  glGenTextures(1, &texID);
  glBindTexture(GL_TEXTURE_2D, texID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

  free(data);
}

Texture::Texture(Texture&& tex) : Texture(0, 0)
{
  swap(*this, tex);
}

Texture::~Texture()
{
  if (!rendererInitialized)
  {
    fprintf(stderr, "Renderer not initialized\n");
    exit(-1);
  }

  glDeleteTextures(1, &texID);
}

Texture& Texture::operator= (Texture tex)
{
  swap(*this, tex);

  return *this;
}

void swap(Texture& tex1, Texture& tex2)
{
  std::swap(tex1.width, tex2.width);
  std::swap(tex1.height, tex2.height);
  std::swap(tex1.texID, tex2.texID);
}

void Texture::fill(Rectangle dstrect, int r, int g, int b)
{
  if (!rendererInitialized)
  {
    fprintf(stderr, "Renderer not initialized\n");
    exit(-1);
  }

  // Target the framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, generic_framebuffer);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texID, 0);

  // Set up the vertex attributes
  GLfloat minx = (GLfloat) dstrect.x / width * 2.0 - 1.0;
  GLfloat miny = -((GLfloat) dstrect.y / height * 2.0 - 1.0);
  GLfloat maxx = (GLfloat) (dstrect.x + dstrect.w) / width * 2.0 - 1.0;
  GLfloat maxy = -((GLfloat) (dstrect.y + dstrect.h) / height * 2.0 - 1.0);

  GLfloat vertexbuffer_data[] = {
    minx, miny,
    maxx, miny,
    maxx, maxy,
    minx, miny,
    minx, maxy,
    maxx, maxy
  };
  GLuint vertexbuffer;
  glGenBuffers(1, &vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertexbuffer_data), vertexbuffer_data, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

  glViewport(0, 0, width, height);
  glClear(GL_DEPTH_BUFFER_BIT);
  glUseProgram(fillShader);
  glUniform3f(glGetUniformLocation(fillShader, "vecColor"), r / 255.0, g / 255.0, b / 255.0);

  glDrawArrays(GL_TRIANGLES, 0, 6);

  glDisableVertexAttribArray(0);
  glDeleteBuffers(1, &vertexbuffer);
}

void Texture::blit(const Texture& srctex, Rectangle srcrect, Rectangle dstrect, double alpha)
{
  if (!rendererInitialized)
  {
    fprintf(stderr, "Renderer not initialized\n");
    exit(-1);
  }

  alpha = glm::clamp(alpha, 0.0, 1.0);

  // Target the framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, generic_framebuffer);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texID, 0);

  // Set up the vertex attributes
  GLfloat minx = (GLfloat) dstrect.x / width * 2.0 - 1.0;
  GLfloat miny = -((GLfloat) dstrect.y / height * 2.0 - 1.0);
  GLfloat maxx = (GLfloat) (dstrect.x + dstrect.w) / width * 2.0 - 1.0;
  GLfloat maxy = -((GLfloat) (dstrect.y + dstrect.h) / height * 2.0 - 1.0);

  GLfloat vertexbuffer_data[] = {
    minx, miny,
    maxx, miny,
    minx, maxy,
    maxx, maxy
  };
  GLuint vertexbuffer;
  glGenBuffers(1, &vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertexbuffer_data), vertexbuffer_data, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

  GLfloat minu = (GLfloat) srcrect.x / srctex.width;
  GLfloat minv = 1 - ((GLfloat) srcrect.y / srctex.height);
  GLfloat maxu = (GLfloat) (srcrect.x + srcrect.w) / srctex.width;
  GLfloat maxv = 1 - ((GLfloat) (srcrect.y + srcrect.h) / srctex.height);

  GLfloat texcoordbuffer_data[] = {
    minu, minv,
    maxu, minv,
    minu, maxv,
    maxu, maxv
  };
  GLuint texcoordbuffer;
  glGenBuffers(1, &texcoordbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, texcoordbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(texcoordbuffer_data), texcoordbuffer_data, GL_STATIC_DRAW);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

  // Set up the shader
  glUseProgram(blitShader);
  glClear(GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, width, height);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, srctex.texID);
  glUniform1i(glGetUniformLocation(blitShader, "srctex"), 0);
  glUniform1f(glGetUniformLocation(blitShader, "alpha"), alpha);

  // Blit!
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  // Unload everything
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(0);
  glDeleteBuffers(1, &texcoordbuffer);
  glDeleteBuffers(1, &vertexbuffer);
}

void bloomPass1(GLuint srcTex, GLuint dstTex, bool horizontal, glm::vec2 srcRes, glm::vec2 dstRes)
{
  glBindFramebuffer(GL_FRAMEBUFFER, generic_framebuffer);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, dstTex, 0);
  glViewport(0,0,dstRes.x,dstRes.y);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(bloom1Shader);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, srcTex);
  glUniform1i(glGetUniformLocation(bloom1Shader, "inTex"), 0);

  glm::vec2 offset = glm::vec2(0.0);
  if (horizontal)
  {
    offset.x = 1.2/srcRes.x;
  } else {
    offset.y = 1.2/srcRes.y;
  }

  glUniform2f(glGetUniformLocation(bloom1Shader, "offset"), offset.x, offset.y);

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glDisableVertexAttribArray(0);
}

void Texture::renderScreen() const
{
  if (!rendererInitialized)
  {
    fprintf(stderr, "Renderer not initialized\n");
    exit(-1);
  }

  // First we're going to composite our frame with the previous frame
  // We start by setting up the framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, generic_framebuffer);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexBufs[curBuf], 0);

  // Set up the shader
  glViewport(0,0,GAME_WIDTH,GAME_HEIGHT);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(ntscShader);

  // Use the current frame texture, nearest neighbor and clamped to edge
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glUniform1i(glGetUniformLocation(ntscShader, "curFrameSampler"), 0);

  // Use the previous frame composite texture, nearest neighbor and clamped to edge
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, renderedTexBufs[(curBuf + 1) % 2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glUniform1i(glGetUniformLocation(ntscShader, "prevFrameSampler"), 1);

  // Load the NTSC artifact texture
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, artifactsTex);
  glUniform1i(glGetUniformLocation(ntscShader, "NTSCArtifactSampler"), 2);
  glUniform1f(glGetUniformLocation(ntscShader, "NTSCLerp"), curBuf * 1.0);

  if ((rand() % 60) == 0)
  {
    // Change the 0.0 to a 1.0 or a 10.0 for a glitchy effect!
    glUniform1f(glGetUniformLocation(ntscShader, "Tuning_NTSC"), 0.0);
  } else {
    glUniform1f(glGetUniformLocation(ntscShader, "Tuning_NTSC"), 0.0);
  }

  // Render our composition
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glDisableVertexAttribArray(0);

  // We're going to render the screen now
  glBindFramebuffer(GL_FRAMEBUFFER, bloom_framebuffer);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, preBloomTex, 0);
  glViewport(0,0,buffer_width,buffer_height);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(finalShader);

  // Use the composited frame texture, linearly filtered and filling in black for the border
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, renderedTexBufs[curBuf]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  float border_color[] = {0.0f, 0.0f, 0.0f, 1.0f};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
	glGenerateMipmap(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(finalShader, "rendertex"), 0);

  // Use the scanlines texture
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, scanlinesTex);
  glUniform1i(glGetUniformLocation(finalShader, "scanlinestex"), 1);

  // Initialize the MVP matrices
  glm::mat4 p_matrix = glm::perspective(42.5f, (float) buffer_width / (float) buffer_height, 0.1f, 100.0f);
  glm::mat4 v_matrix = glm::lookAt(glm::vec3(2,0,0), glm::vec3(0,0,0), glm::vec3(0,1,0));
  glm::mat4 m_matrix = glm::mat4(1.0);
  glm::mat4 mvp_matrix = p_matrix * v_matrix * m_matrix;

  glUniformMatrix4fv(glGetUniformLocation(finalShader, "MVP"), 1, GL_FALSE, &mvp_matrix[0][0]);
  glUniformMatrix4fv(glGetUniformLocation(finalShader, "worldMat"), 1, GL_FALSE, &m_matrix[0][0]);
  glUniform2f(glGetUniformLocation(finalShader, "resolution"), buffer_width, buffer_height);
  glUniform1f(glGetUniformLocation(finalShader, "iGlobalTime"), glfwGetTime());

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, mesh_vertexbuffer);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, mesh_normalbuffer);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, mesh_uvbuffer);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

  glDrawArrays(GL_TRIANGLES, 0, mesh_numvertices);
  glDisableVertexAttribArray(2);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(0);

  // First pass of bloom!
  glm::vec2 buffer_size = glm::vec2(buffer_width, buffer_height);
  bloomPass1(preBloomTex, bloomPassTex1, true, buffer_size, buffer_size / 4.0f);
  bloomPass1(bloomPassTex1, bloomPassTex2, false, buffer_size / 4.0f, buffer_size / 4.0f);

  // Do the second pass of bloom and render to screen
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, buffer_width, buffer_height);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(bloom2Shader);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, preBloomTex);
  glUniform1i(glGetUniformLocation(bloom2Shader, "clearTex"), 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, bloomPassTex2);
  glUniform1i(glGetUniformLocation(bloom2Shader, "blurTex"), 1);

  glUniform1f(glGetUniformLocation(bloom2Shader, "iGlobalTime"), glfwGetTime());

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glDisableVertexAttribArray(0);

  glfwSwapBuffers(window);

  curBuf = (curBuf + 1) % 2;
}

Rectangle Texture::entirety() const
{
  return {0, 0, width, height};
}
