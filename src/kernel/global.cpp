#include "../../lib/hw.h"

extern "C" void memcpy(void* dest, const void* src, size_t n) {
    uint8* d = (uint8*)dest, *s = (uint8*)src;

    for (size_t i = 0; i < n; i++)
        *(d++) = *(s++);
}