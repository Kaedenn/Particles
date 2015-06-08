
#include "Debug.h"

#include <sstream>

namespace Extra {
namespace Debug {
    static DebugLevel _level;

    void Enable(DebugLevel level) {
        _level = level;
    }

    size_t Print(const DebugLevel& desired, const std::string& file, int line, const std::string& message) {
        size_t ret = 0;
        if (desired <= _level) {
            std::ostringstream oss;
            std::string finalMessage;
            oss << file << ":" << line << ": " << message << std::endl;
            finalMessage = oss.str();
            ret = finalMessage.size();
            std::cerr << finalMessage;
        }
        return ret;
    }

}
}
