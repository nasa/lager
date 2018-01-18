#ifndef LAGER_UTILS
#define LAGER_UTILS

#include <iostream>
#include <map>
#include <sstream>

#ifdef _WIN32
// for uuid in Windows
#pragma comment(lib, "rpcrt4.lib")
#else
#include <uuid/uuid.h>
#include <unistd.h>
#endif

#include <zmq.hpp>

namespace lager_utils
{
    // prints the current hashmap to stdout
    static void printHashMap(const std::map<std::string, std::string>& map)
    {
        std::cout << "{";

        for (auto i = map.begin(); i != map.end();)
        {
            std::cout << "'" << i->first << "':'" << i->second << "'";

            ++i;

            if (i != map.end())
            {
                std::cout << ", ";
            }
        }

        std::cout << "}" << std::endl;
    }

    // cross platform sleep
    static void sleep(int ms)
    {
#ifdef _WIN32
        Sleep(ms);
#else
        usleep(ms * 1000);
#endif
    }

    // generates a unique id in a cross platform way
    static std::string getUuid()
    {
#ifdef _WIN32
        UUID uuid;
        char* uuidCstr;
        std::string uuidStr;

        UuidCreate(&uuid);
        UuidToStringA(&uuid, (RPC_CSTR*)&uuidCstr);
        uuidStr = std::string(uuidCstr);
        RpcStringFreeA((RPC_CSTR*)&uuidCstr);
        return uuidStr;
#else
        uuid_t uuid;
        uuid_generate(uuid);
        return std::string(std::begin(uuid), std::end(uuid));
#endif
    }

    // helper to format uris
    static std::string getLocalUri(int port)
    {
        std::stringstream ss;
        ss << "tcp://*:" << port;
        return ss.str();
    }

    // helper to format uris
    static std::string getRemoteUri(const std::string& remoteUriBase, int remotePort)
    {
        std::stringstream ss;
        ss << "tcp://" << remoteUriBase << ":" << remotePort;
        return ss.str();
    }

    // grab zmq version for debugging purposes
    static std::string getZmqVersion()
    {
        std::stringstream ss;
        int major, minor, patch;

        zmq_version(&major, &minor, &patch);
        ss << major << "." << minor << "." << patch;

        return ss.str();
    }
}

#endif
