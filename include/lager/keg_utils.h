#include <iostream>
#include <string>
#include <sys/stat.h> // stat
#include <errno.h>    // errno, ENOENT, EEXIST

namespace keg_utils
{
    /**
    * @brief Cross platform directory checker
    */
    // https://stackoverflow.com/a/29828907/1544725
    static bool isDir(const std::string& path)
    {
#ifdef _WIN32
        struct _stat info;

        if (_stat(path.c_str(), &info) != 0)
        {
            return false;
        }

        return (info.st_mode & _S_IFDIR) != 0;
#else
        struct stat info;

        if (stat(path.c_str(), &info) != 0)
        {
            return false;
        }

        return (info.st_mode & S_IFDIR) != 0;
#endif
    }

    static bool doesFileExist(const std::string& path)
    {
#ifdef _WIN32
        struct _stat info;

        return (_stat(path.c_str(), &info) == 0);
#else
        struct stat info;

        return (stat(path.c_str(), &info) == 0);
#endif
    }
}
