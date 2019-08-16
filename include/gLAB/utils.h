#pragma once

#include "../lib/glabexternal.h"

#include <cstdint>
#include <cstdio>

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

inline bool checkAvailTransientBuffers(uint32_t _numVertices,
                                       const bgfx::VertexDecl& _decl,
                                       uint32_t _numIndices) {
    return _numVertices ==
               bgfx::getAvailTransientVertexBuffer(_numVertices, _decl) &&
           (0 == _numIndices ||
            _numIndices == bgfx::getAvailTransientIndexBuffer(_numIndices));
}
