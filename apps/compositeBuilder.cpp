#include <stdlib.h>
#include <string.h>
#include <string>
#include "AdaboostClassifier.h"
#include "CompositeClassifier.h"

using std::string;

int main(int argc, char* argv[])
{
  int i;
  CompositeClassifier compositeClassifier;
  AdaboostClassifier* adaboostClassifier;
  char* filename;
  char* compositeFilename;
  string className;

  if ( (argc < 4) || (argc % 2 != 0) )
  {
    printf("Usage: %s <composite classifier file> <class name> <classifier file> [ <class name> <classifier file> ... ]\n", argv[0]);
    return 1;
  }

  compositeFilename = argv[1];
  for (i = 2; i < argc; i += 2)
  {
    className = argv[i];
    filename = argv[i + 1];
    adaboostClassifier = new AdaboostClassifier;
    if ( !adaboostClassifier->Load(filename) )
    {
      fprintf(stderr, "Error loading %s\n", filename);
      return 1;
    }
    if ( !compositeClassifier.AddClassifier(adaboostClassifier, className) )
    {
      fprintf(stderr, "Error adding classifier from %s\n", filename);
      return 1;
    }
  }
  compositeClassifier.Save(compositeFilename);

  return 0;
}