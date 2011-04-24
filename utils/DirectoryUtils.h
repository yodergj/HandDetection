#ifndef DIRECTORY_UTILS_H
#define DIRECTORY_UTILS_H

#include <string>
#include <stdio.h>

using std::string;

string GetCurrentDir();
bool MakeDirectory(string directoryName);
bool ChangeDirectory(string directoryName);
bool IsADirectory(string location);
bool FileExists(string filename);
string GetFilenamePart(string path);
string GetDirectoryPart(string path);
string GetCleanedPath(string path);
string Readline(FILE* fp, bool& eof);
string ReadFile(const char* filename);
int ReadInt(FILE* fp, bool& error);

#endif