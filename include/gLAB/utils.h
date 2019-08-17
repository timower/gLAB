#pragma once

#include "../lib/glabexternal.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>

#define ASSERT(cond, msg) \
    do {                  \
        if (!(cond)) {    \
            die(msg);     \
        }                 \
    } while (0)

/**
 * @brief die Exits program after printing \p reason to stderr.
 * @param reason The reason for the exit
 */
[[noreturn]] inline void die(const char* reason) {
    fprintf(stderr, "%s\n", reason);
    std::exit(-1);
}
