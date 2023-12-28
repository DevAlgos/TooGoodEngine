#pragma once
#include <Utils/Logger.h>

#ifdef NDEBUG
#define ASSERT(condition, message) ((void)0)
#else
#define ENGINE_BREAK() __debugbreak()

#define ENGINE_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            LOG_CORE_ERROR(#condition + "' failed: " + (message)) \
            ENGINE_BREAK(); \
        } \
    } while (false)
#endif