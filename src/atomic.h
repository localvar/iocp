#pragma once

#include <stdatomic.h>

typedef struct ATOMIC_SLIST_NODE
{
	struct ATOMIC_SLIST_NODE* next;
} ATOMIC_SLIST_NODE;

#ifdef _MSC_VER

#ifdef _WIN64

typedef union __declspec(align(16)) ATOMIC_SLIST_HEAD
{
    __int64 alignment[2];
    struct
    {
        ATOMIC_SLIST_NODE* next;
        unsigned long long cnt;
    };
} ATOMIC_SLIST_HEAD;

#else   // _WIN64

typedef union __declspec(align(8)) ATOMIC_SLIST_HEAD
{
    __int64 alignment;
    atomic_uint alignment;
    struct
    {
        ATOMIC_SLIST_NODE* next;
        unsigned long cnt;
    };
} ATOMIC_SLIST_HEAD;

#endif // _WIN64

#elif __GNUC__

#ifdef __x86_64__

typedef union __attribute__((aligned(16))) ATOMIC_SLIST_HEAD
{
    __int128 alignment;
    struct
    {
        ATOMIC_SLIST_NODE* next;
        unsigned long int cnt;
    };
} ATOMIC_SLIST_HEAD;

#else   // __x86_64

typedef union __attribute__((aligned(8))) ATOMIC_SLIST_HEAD
{
    __int64 alignment;
    struct
    {
        ATOMIC_SLIST_NODE* next;
        unsigned int cnt;
    };
} ATOMIC_SLIST_HEAD;

#endif  // __x86_64

#endif


void atomic_slist_init( ATOMIC_SLIST_HEAD* head );
void atomic_slist_push( ATOMIC_SLIST_HEAD* head, ATOMIC_SLIST_NODE* node );
ATOMIC_SLIST_NODE* atomic_slist_pop( ATOMIC_SLIST_HEAD* head );
