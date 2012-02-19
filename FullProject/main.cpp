#include <stdio.h>
#include <string>
using std::string;

int main(int argc, char* argv[])
{
  string inputFilename, outputFilename;
  if ( argc < 3 )
  {
    printf("Usage: %s <input file> <output file>\n", argv[0]);
    return 1;
  }
  inputFilename = argv[1];
  outputFilename = argv[2];

  return 0;
}
