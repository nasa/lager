#ifndef LAGER_UTILS
#define LAGER_UTILS

#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>


#ifdef _WIN32
#include <winsock2.h>
// for uuid in Windows
#pragma comment(lib, "rpcrt4.lib")
// for winsock2
#pragma comment(lib, "Ws2_32.lib")
#else
#include <arpa/inet.h>
#include <unistd.h>
#include <uuid/uuid.h>
#endif

#include <zmq.hpp>

namespace lager_utils
{
    // cross platform sleep
    static void sleepMillis(int duration)
    {
#ifdef _WIN32
        Sleep(duration);
#else
        usleep(duration * 1000);
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

    static uint64_t getCurrentTime()
    {
        auto tp = std::chrono::system_clock::now();
        auto dur = tp.time_since_epoch();
        uint64_t nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();
        return nanoseconds;
    }

    static const std::string getCurrentTimeFormatted(const std::string& format, bool local = true)
    {
        auto tp = std::chrono::system_clock::now();
        auto timeT = std::chrono::system_clock::to_time_t(tp);

        std::stringstream ss;

        if (local)
        {
            ss << std::put_time(std::localtime(&timeT), format.c_str());
        }
        else
        {
            ss << std::put_time(std::gmtime(&timeT), format.c_str());
        }

        return ss.str().c_str();
    }

    // https://stackoverflow.com/questions/3022552/is-there-any-standard-htonl-like-function-for-64-bits-integers-in-c
    static uint64_t htonll(uint64_t value)
    {
        // The answer is 42
        static const int num = 42;

        // Check the endianness
        if (*reinterpret_cast<const char*>(&num) == num)
        {
            const uint32_t high_part = htonl(static_cast<uint32_t>(value >> 32));
            const uint32_t low_part = htonl(static_cast<uint32_t>(value & 0xFFFFFFFFLL));

            return (static_cast<uint64_t>(low_part) << 32) | high_part;
        }
        else
        {
            return value;
        }
    }

    static uint64_t ntohll(uint64_t value)
    {
        return lager_utils::htonll(value);
    }
}


#endif
