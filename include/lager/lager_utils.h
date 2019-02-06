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

#include "lager_defines.h"

namespace lager_utils
{
    /**
    * @brief Cross platform sleep function
    * @param duration is the number of milliseconds to sleep for
    */
    static void sleepMillis(int duration)
    {
#ifdef _WIN32
        Sleep(duration);
#else
        usleep(duration * 1000);
#endif
    }

#ifdef _WIN32
    /**
    * @brief Windows only helper function to translate a uuid into a human readable string
    * @param uuid is a Windows UUID struct to translate
    * @returns a string containing the human readable uuid
    */
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

    /**
    * @brief Generates a 16 byte uuid
    * @returns a string containing the 16 byte uuid
    */
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

    /**
    * @brief Translates the given human readable string into a 16 byte uuid string
    * @param uuidStr is a string containing the human readable string of the standard uuid format
    * @returns a 16 byte uuid string
    */
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

    /**
     * @brief Gets the human readable string from the given 16 byte uuid string
     * @param uuidIn is a 16 byte uuid string of the standard uuid format
     * @returns a string in the human readable standard uuid format
     */
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

    /**
    * @brief Helper to get a local zmq uri
    * @param port is a port number generate the uri with
    * @returns a string containing the uri
    */
    static std::string getLocalUri(int port)
    {
        std::stringstream ss;
        ss << "tcp://*:" << port;
        return ss.str();
    }

    /**
    * @brief Helper to get a remote zmq uri
    * @param remoteUriBase is a string with the IP address or hostname to generate the uri with
    * @param remotePort is a port number generate the uri with
    * @returns a string containing the uri
    */
    static std::string getRemoteUri(const std::string& remoteUriBase, int remotePort)
    {
        std::stringstream ss;
        ss << "tcp://" << remoteUriBase << ":" << remotePort;
        return ss.str();
    }

    /**
    * @brief Get the zmq version
    * @returns a string containing the major.minor.patch zmq version
    */
    static std::string getZmqVersion()
    {
        std::stringstream ss;
        int major, minor, patch;

        zmq_version(&major, &minor, &patch);
        ss << major << "." << minor << "." << patch;

        return ss.str();
    }

    /**
    * @brief Gets the current system time as a 64 bit epoch nanoseconds
    * @returns a uint64 containing the system's epoch in nanoseconds
    */
    static uint64_t getCurrentTime()
    {
        auto tp = std::chrono::system_clock::now();
        auto dur = tp.time_since_epoch();
        uint64_t nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();
        return nanoseconds;
    }

    /**
    * @brief Gets a formatted string containing the current system time
    * @param format is a standard strftime format specifier
    * @param local is an option boolean parameter specifying whether the time should be local or gmt (defaults true)
    * @returns a string containing the current system time formatted as per the input parameters
    */
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
            strftime(outputBuffer, sizeof(outputBuffer), format.c_str(), &timeInfo);
            ss << outputBuffer;
        }
        else
        {
#ifdef _WIN32
            gmtime_s(&timeInfo, &timeT);
#else
            gmtime_r(&timeT, &timeInfo);
#endif
            strftime(outputBuffer, sizeof(outputBuffer), format.c_str(), &timeInfo);
            ss << outputBuffer;
        }

        return ss.str();
    }

    /**
    * @brief 64 bit host to network byte order translator
    * @param value is a uint64 in host order
    * @returns a uint64 in network byte order
    * https://stackoverflow.com/questions/3022552/is-there-any-standard-htonl-like-function-for-64-bits-integers-in-c
    */
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

    /**
    * @brief 64 bit network to host byte order translator
    * @param value is a uint64 in network order
    * @returns a uint64 in host byte order
    */
    static uint64_t ntohll(uint64_t value)
    {
        return lager_utils::htonll(value);
    }
}

#endif
