#ifndef LAGER_UTILS
#define LAGER_UTILS

#include <chrono>
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

#ifdef _WIN32
    static std::string getBytesFromWindowsUuid(UUID uuid)
    {
        const unsigned char uuidOut[UUID_SIZE_BYTES] =
        {
            // these static casts were necessary to avoid narrowing warnings
            static_cast<unsigned char>((uuid.Data1 >> 24) & 0xff),
            static_cast<unsigned char>((uuid.Data1 >> 16) & 0xff),
            static_cast<unsigned char>((uuid.Data1 >> 8) & 0xff),
            static_cast<unsigned char>((uuid.Data1) & 0xff),

            static_cast<unsigned char>((uuid.Data2 >> 8) & 0xff),
            static_cast<unsigned char>((uuid.Data2) & 0xff),

            static_cast<unsigned char>((uuid.Data3 >> 8) & 0xff),
            static_cast<unsigned char>((uuid.Data3) & 0xff),

            static_cast<unsigned char>(uuid.Data4[0]),
            static_cast<unsigned char>(uuid.Data4[1]),
            static_cast<unsigned char>(uuid.Data4[2]),
            static_cast<unsigned char>(uuid.Data4[3]),
            static_cast<unsigned char>(uuid.Data4[4]),
            static_cast<unsigned char>(uuid.Data4[5]),
            static_cast<unsigned char>(uuid.Data4[6]),
            static_cast<unsigned char>(uuid.Data4[7])
        };

        return std::string(uuidOut, uuidOut + UUID_SIZE_BYTES);
    }
#endif

    // generates a unique id in a cross platform way
    static std::string getUuid()
    {
#ifdef _WIN32
        UUID uuid;
        UuidCreate(&uuid);
        return getBytesFromWindowsUuid(uuid);
#else
        uuid_t uuid;
        uuid_generate(uuid);
        return std::string(std::begin(uuid), std::end(uuid));
#endif
    }

    // gets a uuid from a human readable string
    static std::string getUuid(const std::string& uuidStr)
    {
#ifdef _WIN32
        UUID uuid;
        UuidFromStringA((RPC_CSTR)uuidStr.c_str(), &uuid);
        return getBytesFromWindowsUuid(uuid);
#else
        uuid_t uuid;
        uuid_parse(uuidStr.c_str(), uuid);
        return std::string(std::begin(uuid), std::end(uuid));
#endif
    }

    static std::string getUuidString(const std::string& uuidIn)
    {
#ifdef _WIN32
        char* uuidCstr;
        std::string uuidStr;

        UUID uuid;

        uuid.Data1 =  (uuidIn[0] & 0xff) << 24;
        uuid.Data1 += (uuidIn[1] & 0xff) << 16;
        uuid.Data1 += (uuidIn[2] & 0xff) << 8;
        uuid.Data1 += (uuidIn[3] & 0xff);

        uuid.Data2 = (uuidIn[4] & 0xff) << 8;
        uuid.Data2 += uuidIn[5] & 0xff;

        uuid.Data3 = (uuidIn[6] & 0xff) << 8;
        uuid.Data3 += uuidIn[7] & 0xff;

        uuid.Data4[0] = uuidIn[8 + 0];
        uuid.Data4[1] = uuidIn[8 + 1];
        uuid.Data4[2] = uuidIn[8 + 2];
        uuid.Data4[3] = uuidIn[8 + 3];
        uuid.Data4[4] = uuidIn[8 + 4];
        uuid.Data4[5] = uuidIn[8 + 5];
        uuid.Data4[6] = uuidIn[8 + 6];
        uuid.Data4[7] = uuidIn[8 + 7];

        UuidToStringA(&uuid, (RPC_CSTR*)&uuidCstr);
        uuidStr = std::string(uuidCstr);
        RpcStringFreeA((RPC_CSTR*)&uuidCstr);
        return uuidStr;
#else
        char uuidStr[37];
        uuid_t uuid;

        for (unsigned int i = 0; i < UUID_SIZE_BYTES; ++i)
        {
            uuid[i] = uuidIn[i];
        }

        uuid_unparse_lower(uuid, uuidStr);
        return std::string(std::begin(uuidStr), std::end(uuidStr));
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

        char outputBuffer[500];
        std::stringstream ss;
        struct tm timeInfo;

        if (local)
        {
#ifdef _WIN32
            _tzset();
            localtime_s(&timeInfo, &timeT);
#else
            tzset();
            localtime_r(&timeT, &timeInfo);
#endif
            strftime(outputBuffer, 500, format.c_str(), &timeInfo);
            ss << outputBuffer;
        }
        else
        {
#ifdef _WIN32
            gmtime_s(&timeInfo, &timeT);
#else
            gmtime_r(&timeT, &timeInfo);
#endif
            strftime(outputBuffer, 500, format.c_str(), &timeInfo);
            ss << outputBuffer;
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
