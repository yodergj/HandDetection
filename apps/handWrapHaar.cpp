#include <string.h>
#include <string>
#include "FleshDetector.h"
#include "HandDetector.h"
#include "Image.h"

using std::string;

#ifndef MAX
#define MAX(a,b) ( (a) > (b) ? (a) : (b) )
#endif

int main(int argc, char* argv[])
{
  char filename[256];
  FILE *file;
  int revNumber = 0;
  Image image;
  HandDetector detector;

  if ( argc != 2 )
  {
    printf("Usage: %s <haar cascade file>\n", argv[0]);
    return 0;
  }

  if ( !detector.Create(argv[1]) )
  {
    printf("Failed creating detector\n");
    return 1;
  }

  do
  {
    sprintf(filename, "hand-haar.rev%d.cfg", revNumber);
    file = fopen(filename, "r");
    if ( file )
      fclose(file);
    revNumber++;
  } while ( file );

  detector.Save(filename);

  return 0;
}