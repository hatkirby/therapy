#include "renderer.h"
#include <string>
#include <fstream>
#include <vector>
#include <cstdio>
#include "map.h"

static bool rendererInitialized = false;

static GLFWwindow* window;

static GLuint FramebufferName; // The framebuffer
static GLuint ntscShader; // The NTSC shader
static GLuint finalShader; // The passthrough shader
static GLuint blitShader; // The blitting shader
static GLuint fillShader; // The fill shader

// The buffers for the NTSC rendering process
static GLuint renderedTex1;
static GLuint renderedTex2;
static GLuint renderedTexBufs[2];
static int curBuf;
static GLuint artifactsTex;
static GLuint scanlinesTex;

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
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);
 
    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> VertexShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);
 
    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);
 
    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
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
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);
 
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);
 
    return ProgramID;
}

GLuint loadBMP_custom(const char * imagepath){

	printf("Reading image %s\n", imagepath);

	// Data read from the header of the BMP file
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	unsigned int width, height;
	// Actual RGB data
	unsigned char * data;

	// Open the file
	FILE * file = fopen(imagepath,"rb");
	if (!file)							    {printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); getchar(); return 0;}

	// Read the header, i.e. the 54 first bytes

	// If less than 54 bytes are read, problem
	if ( fread(header, 1, 54, file)!=54 ){ 
		printf("Not a correct BMP file\n");
		return 0;
	}
	// A BMP files always begins with "BM"
	if ( header[0]!='B' || header[1]!='M' ){
		printf("Not a correct BMP file\n");
		return 0;
	}
	// Make sure this is a 24bpp file
	if ( *(int*)&(header[0x1E])!=0  )         {printf("Not a correct BMP file\n");    return 0;}
	if ( *(int*)&(header[0x1C])!=24 )         {printf("Not a correct BMP file\n");    return 0;}

	// Read the information about the image
	dataPos    = *(int*)&(header[0x0A]);
	imageSize  = *(int*)&(header[0x22]);
	width      = *(int*)&(header[0x12]);
	height     = *(int*)&(header[0x16]);

	// Some BMP files are misformatted, guess missing information
	if (imageSize==0)    imageSize=width*height*3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos==0)      dataPos=54; // The BMP header is done that way

	// Create a buffer
	data = new unsigned char [imageSize];

	// Read the actual data from the file into the buffer
	fread(data,1,imageSize,file);

	// Everything is in memory now, the file wan be closed
	fclose (file);

	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);
	
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	// OpenGL has now copied the data. Free our own version
	delete [] data;

	// Poor filtering, or ...
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 

	// ... nice trilinear filtering.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
	glGenerateMipmap(GL_TEXTURE_2D);

	// Return the ID of the texture we just created
	return textureID;
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
  window = glfwCreateWindow(1024, 768, "Aromatherapy", NULL, NULL);
  if (window == NULL)
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
  
  // Set up vertex array object
  GLuint VertexArrayID;
  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);
  
  // Set up the framebuffer
  glGenFramebuffers(1, &FramebufferName);
  glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
  GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, DrawBuffers);
  
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
  
  curBuf = 0;
  
  artifactsTex = loadBMP_custom("../res/artifacts.bmp");
  scanlinesTex = loadBMP_custom("../res/scanlines.bmp");
  
  // Load the shaders
  ntscShader = LoadShaders("../shaders/ntsc.vertex", "../shaders/ntsc.fragment");
  finalShader = LoadShaders("../shaders/final.vertex", "../shaders/final.fragment");
  blitShader = LoadShaders("../shaders/blit.vertex", "../shaders/blit.fragment");
  fillShader = LoadShaders("../shaders/fill.vertex", "../shaders/fill.fragment");
  
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
  
  // Delete the shaders
  glDeleteProgram(ntscShader);
  glDeleteProgram(finalShader);
  glDeleteProgram(blitShader);
  glDeleteProgram(fillShader);
  
  // Delete the NTSC rendering buffers
  glDeleteTextures(1, &renderedTex1);
  glDeleteTextures(1, &renderedTex2);
  glDeleteTextures(1, &artifactsTex);
  glDeleteTextures(1, &scanlinesTex);
  
  // Delete the framebuffer
  glDeleteFramebuffers(1, &FramebufferName);
  
  // Kill the window
  glfwTerminate();
  
  rendererInitialized = false;
}

Texture* createTexture(int width, int height)
{
  if (!rendererInitialized)
  {
    fprintf(stderr, "Renderer not initialized\n");
    exit(-1);
  }
  
  Texture* tex = new Texture();
  tex->width = width;
  tex->height = height;
  
  glGenTextures(1, &(tex->texID));
  glBindTexture(GL_TEXTURE_2D, tex->texID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  
  return tex;
}

void destroyTexture(Texture* tex)
{
  if (!rendererInitialized)
  {
    fprintf(stderr, "Renderer not initialized\n");
    exit(-1);
  }
  
  glDeleteTextures(1, &(tex->texID));
  
  delete tex;
}

Texture* loadTextureFromBMP(char* filename)
{
  if (!rendererInitialized)
  {
    fprintf(stderr, "Renderer not initialized\n");
    exit(-1);
  }
  
  Texture* tex = new Texture();
  tex->texID = loadBMP_custom(filename);
  
  glBindTexture(GL_TEXTURE_2D, tex->texID);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &(tex->width));
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &(tex->height));
  
  return tex;
}

void saveTextureToBMP(Texture* tex, char* filename)
{
  if (!rendererInitialized)
  {
    fprintf(stderr, "Renderer not initialized\n");
    exit(-1);
  }
  
  int size = 54 + 3*tex->width*tex->height;
  
  char* buf = (char*) calloc(size, sizeof(char));
  buf[0x00] = 'B';
  buf[0x01] = 'M';
  *(int*)&(buf[0x0A]) = 54;
  *(int*)&(buf[0x12]) = tex->width;
  *(int*)&(buf[0x16]) = tex->height;
  *(int*)&(buf[0x1C]) = 24;
  *(int*)&(buf[0x1E]) = 0;
  *(int*)&(buf[0x22]) = size;
  
  glBindTexture(GL_TEXTURE_2D, tex->texID);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_UNSIGNED_BYTE, buf + 54);
  
  FILE* f = fopen(filename, "wb");
  fwrite(buf, sizeof(char), size, f);
  fclose(f);
  
  free(buf);
}

void fillTexture(Texture* tex, Rectangle* dstrect, int r, int g, int b)
{
  if (!rendererInitialized)
  {
    fprintf(stderr, "Renderer not initialized\n");
    exit(-1);
  }
  
  // Target the framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex->texID, 0);
  
  // Set up the vertex attributes
  GLfloat minx = (dstrect == NULL) ? 0.0f : dstrect->x;
  GLfloat miny = (dstrect == NULL) ? 0.0f : dstrect->y;
  GLfloat maxx = (dstrect == NULL) ? tex->width : dstrect->x + dstrect->w;
  GLfloat maxy = (dstrect == NULL) ? tex->height : dstrect->y + dstrect->h;
  
  minx = minx / tex->width * 2.0 - 1.0;
  miny = -(miny / tex->height * 2.0 - 1.0);
  maxx = maxx / tex->width * 2.0 - 1.0;
  maxy = -(maxy / tex->height * 2.0 - 1.0);
  
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
  
  glViewport(0, 0, tex->width, tex->height);
  glUseProgram(fillShader);
  glUniform3f(glGetUniformLocation(fillShader, "vecColor"), r / 255.0, g / 255.0, b / 255.0);
  
  glDrawArrays(GL_TRIANGLES, 0, 6);
  
  glDisableVertexAttribArray(0);
  glDeleteBuffers(1, &vertexbuffer);
}

void blitTexture(Texture* srctex, Texture* dsttex, Rectangle* srcrect, Rectangle* dstrect)
{
  if (!rendererInitialized)
  {
    fprintf(stderr, "Renderer not initialized\n");
    exit(-1);
  }
  
  // Target the framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, dsttex->texID, 0);
  
  // Set up the vertex attributes
  GLfloat minx = (dstrect == NULL) ? 0.0f : dstrect->x;
  GLfloat miny = (dstrect == NULL) ? 0.0f : dstrect->y;
  GLfloat maxx = (dstrect == NULL) ? dsttex->width : dstrect->x + dstrect->w;
  GLfloat maxy = (dstrect == NULL) ? dsttex->height : dstrect->y + dstrect->h;
  
  minx = minx / dsttex->width * 2.0 - 1.0;
  miny = -(miny / dsttex->height * 2.0 - 1.0);
  maxx = maxx / dsttex->width * 2.0 - 1.0;
  maxy = -(maxy / dsttex->height * 2.0 - 1.0);
  
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
  
  GLfloat minu = (srcrect == NULL) ? 0.0f : srcrect->x;
  GLfloat minv = (srcrect == NULL) ? 0.0f : srcrect->y;
  GLfloat maxu = (srcrect == NULL) ? srctex->width : srcrect->x + srcrect->w;
  GLfloat maxv = (srcrect == NULL) ? srctex->height : srcrect->y + srcrect->h;
  
  minu = minu / srctex->width;
  minv = 1 - (minv / srctex->height);
  maxu = maxu / srctex->width;
  maxv = 1 - (maxv / srctex->height);
  
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
  glViewport(0, 0, dsttex->width, dsttex->height);
  
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, srctex->texID);
  glUniform1i(glGetUniformLocation(blitShader, "srctex"), 0);
  
  // Blit!
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  
  // Unload everything
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(0);
  glDeleteBuffers(1, &texcoordbuffer);
  glDeleteBuffers(1, &vertexbuffer);
}

void renderScreen(Texture* tex)
{
  if (!rendererInitialized)
  {
    fprintf(stderr, "Renderer not initialized\n");
    exit(-1);
  }
  
  // First we're going to composite our frame with the previous frame
  // We start by setting up the framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexBufs[curBuf], 0);
  
  // Set up the shaer
  glViewport(0,0,GAME_WIDTH,GAME_HEIGHT);
  glClear(GL_COLOR_BUFFER_BIT);
  glUseProgram(ntscShader);
  
  // Use the current frame texture, nearest neighbor and clamped to edge
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex->texID);
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
  
  // Load the vertices of a flat surface
  GLfloat g_quad_vertex_buffer_data[] = { 
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
	};

  GLuint quad_vertexbuffer;
  glGenBuffers(1, &quad_vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);
  
  // Render our composition
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glDisableVertexAttribArray(0);

  // Load the normal vertices of a flat surface
  GLfloat g_norms_data[] = {
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f
  };
  
  GLuint g_norms;
  glGenBuffers(1, &g_norms);
  glBindBuffer(GL_ARRAY_BUFFER, g_norms);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_norms_data), g_norms_data, GL_STATIC_DRAW);
  
  // We're going to output to the window now
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0,0,1024,768);
  glClear(GL_COLOR_BUFFER_BIT);
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
  mat4 p_matrix = perspective(90.0f, 4.0f / 4.0f, 0.1f, 100.0f);
  mat4 v_matrix = lookAt(vec3(0,0,1), vec3(0,0,0), vec3(0,1,0));
  mat4 m_matrix = mat4(1.0f);
  mat4 mvp_matrix = p_matrix * v_matrix * m_matrix;
  
  glUniformMatrix4fv(glGetUniformLocation(finalShader, "MVP"), 1, GL_FALSE, &mvp_matrix[0][0]);
  glUniformMatrix4fv(glGetUniformLocation(finalShader, "worldMat"), 1, GL_FALSE, &m_matrix[0][0]);
  
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, g_norms);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(0);
  
  glfwSwapBuffers(window);
  
  glDeleteBuffers(1, &g_norms);
  glDeleteBuffers(1, &quad_vertexbuffer);
  
  curBuf = (curBuf + 1) % 2;
}
