#include "Utilities.h"

std::string GetCurrentDir()
{
    char buff[FILENAME_MAX]; //create string buffer to hold path
    _getcwd(buff, FILENAME_MAX);
    std::string current_working_dir(buff);
    return current_working_dir;
}

std::string GetDirFromPath(std::string path) 
{
    return path.substr(0, path.find_last_of('/\\'));
}