#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include "DirectoryUtils.h"

#define SLASH_STR "/"

bool MakeDirectory(string directoryName)
{
  string command;
  int retcode;
  int length;

  if ( directoryName == "" )
    return false;
  length = directoryName.length();
  if ( directoryName[length - 1] == '/' )
    directoryName.resize(length - 1);
  retcode = mkdir(directoryName.c_str(),0777);
  if ( retcode == -1 )
  {
    if ( errno == EEXIST )
      return true;
    if ( errno == ENOENT )
    {
      int pos = directoryName.rfind(SLASH_STR);
      directoryName[pos] = 0;
      if ( MakeDirectory(directoryName) )
      {
        directoryName[pos] = '/';
        return MakeDirectory(directoryName);
      }
    }
    return false;
  }
  return true;
}

bool ChangeDirectory(string directoryName)
{
  if ( directoryName == "" )
    return false;

  if ( chdir(directoryName.c_str()) == 0 )
    return true;
  if ( errno == ENOENT )
  {
    if ( MakeDirectory(directoryName) && (chdir(directoryName.c_str())==0) )
      return true;
  }
  return false;
}

string GetCurrentDir()
{
  static char *buf=NULL;
  static size_t size=0;

  if ( !buf )
  {
    size = 256;
    buf = (char *)malloc(size);
  }
  if (!buf)
    exit(1);
  while ( !getcwd(buf,size) )
  {
    if ( errno != ERANGE )
      exit(1);
    size *= 2;
    buf = (char *)realloc(buf,size);
    if ( !buf )
      exit(1);
  }
  return buf;
}

bool IsADirectory(string location)
{
  DIR* dir;
  dir = opendir(location.c_str());
  if ( !dir )
    return false;
  closedir(dir);
  return true;
}

bool FileExists(string filename)
{
  FILE *fp;

  fp = fopen(filename.c_str(),"r");

  if ( !fp )
    return false;

  fclose(fp);
  return true;
}

string GetFilenamePart(string path)
{
  int slashPos;

  slashPos = path.rfind(SLASH_STR);

  if ( slashPos < 0 )
    return path;

  return path.substr(slashPos+1);
}

string GetDirectoryPart(string path)
{
  int slashPos;

  slashPos = path.rfind(SLASH_STR);

  if ( slashPos < 0 )
    return "";

  return path.substr(0,slashPos+1);
}

string GetCleanedPath(string path)
{
  int slashPos;
  int numBackups;
  string front, back, result;

  result = "";
  numBackups = 0;
  back = path;
  slashPos = back.rfind('/');
  while (slashPos >= 0)
  {
    front = back.substr(slashPos+1);
    back = back.substr(0,slashPos);
    if ( front == ".." )
      numBackups++;
    else if ( (front != ".") && (front != "") )
    {
      if ( numBackups )
	numBackups--;
      else
      {
	if ( result != "" )
	  result = "/" + result;
	result = front + result;
      }
    }
    slashPos = back.rfind('/');
  }
  result = "/" + result;
  return result;
}

#define BUFFER_LENGTH 256

string Readline(FILE* fp, bool& eof)
{
  char buf[BUFFER_LENGTH];
  string line;
  int length;

  eof = false;

  if ( !fp )
    return "";

  line = "";
  while ( fgets(buf, BUFFER_LENGTH, fp) )
  {
    line += buf;
    length = line.length();
    if ( (line[length - 1] == '\n') ||
         (line[length - 1] == '\r') )
    {
      while ( (line[length - 1] == '\n') ||
              (line[length - 1] == '\r') )
      {
	line.resize(length - 1);
	length--;
      }
      return line;
    }
  }
  if ( line == "" )
    eof = true;
  return line;
}

string ReadFile(const char* filename)
{
  FILE* fp;
  char buf[BUFFER_LENGTH];
  string contents;

  if ( !filename )
    return "";

  fp = fopen(filename, "r");

  if ( !fp )
    return "";

  while ( fgets(buf, BUFFER_LENGTH, fp) )
    contents += buf;

  fclose(fp);

  return contents;
}

int ReadInt(FILE* fp, bool& error)
{
  bool eof;
  string line;
  char *endChar;
  long value = 0;

  line = Readline(fp,eof);
  if ( eof )
  {
    error = true;
    return 0;
  }
  value = strtol(line.c_str(),&endChar,10);
  if ( line.c_str() == endChar )
    error = true;
  else
    error = false;
  return (int)value;
}