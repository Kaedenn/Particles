
#include "Strings.h"

/* For asprintf */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>

#include <cstdarg>

#include <sstream>

namespace Extra {
    std::string AllocPrintf(const char* message, ...) {
        std::string formatted;
        char* result = NULL;
        int n = 0;
        va_list ap;
        va_start(ap, message);
        n = vasprintf(&result, message, ap);
        va_end(ap);

        if (n > 0) {
            formatted = result;
            free(result);
        } else if (n == -1) {
            int error = errno;
            std::ostringstream oss;
            oss << "FATAL: _AllocPrintf(" << message << ").vasprintf failed "
                << "with error " << error << ": " << strerror(error) << std::endl;
            formatted = oss.str();
        }
        return formatted;
    }
}

