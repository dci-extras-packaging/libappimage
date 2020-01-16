// system
#include <cstring>
#include <vector>
#include <sstream>
#include <iomanip>
#include <iostream>

// local
#include "hashlib.h"
// libraries
extern "C" {
#include "md5.h"
}

namespace appimage {
    namespace utils {
        namespace hashlib {

            std::vector<uint8_t> md5(std::istream& data) {
                Md5Context md5_context;
                Md5Initialise(&md5_context);

                // uint32_t type used to ensure that it's the maximum value
                static const uint32_t chunk_size = 4096;
                std::vector<char> buf(chunk_size);

                uint32_t readCount;
                while (data.read(buf.data(), buf.size()) || (readCount = static_cast<uint32_t>(data.gcount())) != 0)
                    Md5Update(&md5_context, buf.data(), readCount); // feed buffer into checksum calculation

                // Finalise computation
                MD5_HASH checksum;
                Md5Finalise(&md5_context, &checksum);

                // Extract digest string
                std::vector<uint8_t> digest(16);
                memcpy(digest.data(), (const char*) checksum.bytes, 16);

                return digest;
            }

            std::vector<uint8_t> md5(const std::string& data) {
                std::stringstream ss(data);
                return md5(ss);
            }

            std::string toHex(std::vector<uint8_t> digest) {
                std::stringstream stream;
                stream << std::hex << std::setfill('0');
                for (const unsigned char& i : digest)
                    stream << std::setw(2) << static_cast<unsigned>(i);

                return stream.str();
            }
        }
    }
}
