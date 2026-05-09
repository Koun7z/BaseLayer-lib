/*
   SipHash reference C implementation

   Copyright (c) 2016 Jean-Philippe Aumasson <jeanphilippe.aumasson@gmail.com>

   To the extent possible under law, the author(s) have dedicated all copyright
   and related and neighboring rights to this software to the public domain
   worldwide. This software is distributed without any warranty.

   You should have received a copy of the CC0 Public Domain Dedication along
   with
   this software. If not, see
   <http://creativecommons.org/publicdomain/zero/1.0/>.
 */
#include "Base_Hash.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>

/* default: SipHash-2-4 */
#ifndef cROUNDS
#  define cROUNDS 2
#endif
#ifndef dROUNDS
#  define dROUNDS 4
#endif

#define ROTL(x, b) (uint32_t) (((x) << (b)) | ((x) >> (32 - (b))))

#define U32TO8_LE(p, v)             \
    (p)[0] = (uint8_t) ((v));       \
    (p)[1] = (uint8_t) ((v) >> 8);  \
    (p)[2] = (uint8_t) ((v) >> 16); \
    (p)[3] = (uint8_t) ((v) >> 24);

#define U8TO32_LE(p)                                                                                                 \
    (((uint32_t) ((p)[0])) | ((uint32_t) ((p)[1]) << 8) | ((uint32_t) ((p)[2]) << 16) | ((uint32_t) ((p)[3]) << 24))

#define SIPROUND            \
    do                      \
    {                       \
        v0 += v1;           \
        v1  = ROTL(v1, 5);  \
        v1 ^= v0;           \
        v0  = ROTL(v0, 16); \
        v2 += v3;           \
        v3  = ROTL(v3, 8);  \
        v3 ^= v2;           \
        v0 += v3;           \
        v3  = ROTL(v3, 7);  \
        v3 ^= v0;           \
        v2 += v1;           \
        v1  = ROTL(v1, 13); \
        v1 ^= v2;           \
        v2  = ROTL(v2, 16); \
    }                       \
    while(0)


#ifdef DEBUG_SIPHASH
#  include <stdio.h>

#  define TRACE                                           \
      do                                                  \
      {                                                   \
          printf("(%3zu) v0 %08" PRIx32 "\n", inlen, v0); \
          printf("(%3zu) v1 %08" PRIx32 "\n", inlen, v1); \
          printf("(%3zu) v2 %08" PRIx32 "\n", inlen, v2); \
          printf("(%3zu) v3 %08" PRIx32 "\n", inlen, v3); \
      }                                                   \
      while(0)
#else
#  define TRACE
#endif

uint32_t Hash_SipHash_u32(const void* key, size_t len, uint32_t k0, uint32_t k1)
{
    const uint8_t* ni  = (const uint8_t*) key;
    const uint8_t* end = ni + (len & ~((size_t) 3));
    const int left     = len & 3;

    uint32_t v0 = 0;
    uint32_t v1 = 0;
    uint32_t v2 = UINT32_C(0x6c796765);
    uint32_t v3 = UINT32_C(0x74656462);
    uint32_t m;
    uint32_t b = ((uint32_t) len) << 24;

    v3 ^= k1;
    v2 ^= k0;
    v1 ^= k1;
    v0 ^= k0;

    /* Compression */
    for(; ni != end; ni += 4)
    {
        m   = U8TO32_LE(ni);
        v3 ^= m;

        for(int i = 0; i < cROUNDS; ++i)
        {
            SIPROUND;
        }

        v0 ^= m;
    }

    /* Last partial block */
    switch(left)
    {
        case 3:
            b |= ((uint32_t) ni[2]) << 16;
            /* fallthrough */
        case 2:
            b |= ((uint32_t) ni[1]) << 8;
            /* fallthrough */
        case 1:
            b |= ((uint32_t) ni[0]);
            /* fallthrough */
        case 0:
            break;
    }

    v3 ^= b;
    for(int i = 0; i < cROUNDS; ++i)
    {
        SIPROUND;
    }
    v0 ^= b;

    /* Finalization */
    v2 ^= 0xff;
    for(int i = 0; i < dROUNDS; ++i)
    {
        SIPROUND;
    }

    return v1 ^ v3;
}
