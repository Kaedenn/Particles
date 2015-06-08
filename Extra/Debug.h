#ifndef EXTRA_DEBUG_INCLUDED
#define EXTRA_DEBUG_INCLUDED

#include <Extra/Strings.h>

#include <iostream>
#include <string>

namespace Extra {
    namespace Debug {
        enum DebugLevel {
            DEBUG_NONE,     /* Debug level 0 */
            DEBUG_VERBOSE,  /* Debug level 1: traditional verbose */
            DEBUG_DEBUG,    /* Debug level 2: for actual debugging */
            DEBUG_TRACE     /* Debug level 3: for tracing function calls */
        };

        void Enable(DebugLevel level = DEBUG_VERBOSE);
        size_t Print(const DebugLevel& desired, const std::string& file, int line, const std::string& message);
    }
}

#define DEBUG(message) Extra::Debug::Print(Extra::Debug::DEBUG_VERBOSE, __FILE__, __LINE__, (message))
#define DEBUGV(message, ...) Extra::Debug::Print(Extra::Debug::DEBUG_VERBOSE, __FILE__, __LINE__, Extra::AllocPrintf((message), __VA_ARGS__))

#endif
