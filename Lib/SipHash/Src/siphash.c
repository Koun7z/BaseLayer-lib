/*
   SipHash reference C implementation

   Copyright (c) 2012-2022 Jean-Philippe Aumasson
   <jeanphilippe.aumasson@gmail.com>
   Copyright (c) 2012-2014 Daniel J. Bernstein <djb@cr.yp.to>

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

#define ROTL(x, b) (uint64_t) (((x) << (b)) | ((x) >> (64 - (b))))

#define U32TO8_LE(p, v)             \
    (p)[0] = (uint8_t) ((v));       \
    (p)[1] = (uint8_t) ((v) >> 8);  \
    (p)[2] = (uint8_t) ((v) >> 16); \
    (p)[3] = (uint8_t) ((v) >> 24);

#define U64TO8_LE(p, v)                         \
    U32TO8_LE((p), (uint32_t) ((v)));           \
    U32TO8_LE((p) + 4, (uint32_t) ((v) >> 32));

#define U8TO64_LE(p)                                                                                                \
    (((uint64_t) ((p)[0])) | ((uint64_t) ((p)[1]) << 8) | ((uint64_t) ((p)[2]) << 16) | ((uint64_t) ((p)[3]) << 24) \
     | ((uint64_t) ((p)[4]) << 32) | ((uint64_t) ((p)[5]) << 40) | ((uint64_t) ((p)[6]) << 48)                      \
     | ((uint64_t) ((p)[7]) << 56))

#define SIPROUND            \
    do                      \
    {                       \
        v0 += v1;           \
        v1  = ROTL(v1, 13); \
        v1 ^= v0;           \
        v0  = ROTL(v0, 32); \
        v2 += v3;           \
        v3  = ROTL(v3, 16); \
        v3 ^= v2;           \
        v0 += v3;           \
        v3  = ROTL(v3, 21); \
        v3 ^= v0;           \
        v2 += v1;           \
        v1  = ROTL(v1, 17); \
        v1 ^= v2;           \
        v2  = ROTL(v2, 32); \
    }                       \
    while(0)

#ifdef DEBUG_SIPHASH
#  include <stdio.h>

#  define TRACE                                            \
      do                                                   \
      {                                                    \
          printf("(%3zu) v0 %016" PRIx64 "\n", inlen, v0); \
          printf("(%3zu) v1 %016" PRIx64 "\n", inlen, v1); \
          printf("(%3zu) v2 %016" PRIx64 "\n", inlen, v2); \
          printf("(%3zu) v3 %016" PRIx64 "\n", inlen, v3); \
      }                                                    \
      while(0)
#else
#  define TRACE
#endif

uint64_t Hash_SipHash_u64(const void* key, size_t len, uint64_t k0, uint64_t k1)
{
    const uint8_t* ni  = (const uint8_t*) key;
    const uint8_t* end = ni + (len & ~((size_t) 7));
    const int left     = len & 7;

    uint64_t v0 = UINT64_C(0x736f6d6570736575);
    uint64_t v1 = UINT64_C(0x646f72616e646f6d);
    uint64_t v2 = UINT64_C(0x6c7967656e657261);
    uint64_t v3 = UINT64_C(0x7465646279746573);

    uint64_t m;
    uint64_t b = ((uint64_t) len) << 56;

    /* Key injection */
    v3 ^= k1;
    v2 ^= k0;
    v1 ^= k1;
    v0 ^= k0;

    /* Compression */
    for(; ni != end; ni += 8)
    {
        m   = U8TO64_LE(ni);
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
        case 7:
            b |= ((uint64_t) ni[6]) << 48;
            /* fallthrough */
        case 6:
            b |= ((uint64_t) ni[5]) << 40;
            /* fallthrough */
        case 5:
            b |= ((uint64_t) ni[4]) << 32;
            /* fallthrough */
        case 4:
            b |= ((uint64_t) ni[3]) << 24;
            /* fallthrough */
        case 3:
            b |= ((uint64_t) ni[2]) << 16;
            /* fallthrough */
        case 2:
            b |= ((uint64_t) ni[1]) << 8;
            /* fallthrough */
        case 1:
            b |= ((uint64_t) ni[0]);
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

    /* Finalization (64-bit output) */
    v2 ^= 0xff;
    for(int i = 0; i < dROUNDS; ++i)
    {
        SIPROUND;
    }

    return v0 ^ v1 ^ v2 ^ v3;
}
