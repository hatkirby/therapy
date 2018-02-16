#include "util.h"

std::string slurp(std::ifstream& in)
{
  std::stringstream sstr;
  sstr << in.rdbuf();
  return sstr.str();
}

void flipImageData(
  unsigned char* data,
  int width,
  int height,
  int comps)
{
  unsigned char* dataCopy = new unsigned char[width * height * comps];
  memcpy(dataCopy, data, width * height * comps);

  int rowSize = width * comps;

  for (int i = 0; i < height; i++)
  {
    memcpy(
      data + (rowSize * i),
      dataCopy + (rowSize * (height - i - 1)),
      rowSize);
  }

  delete[] dataCopy;
}
