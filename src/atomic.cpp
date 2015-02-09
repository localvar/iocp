#include "atomic.h"
#include <assert.h>
#include <string.h>
#include <stddef.h>



#ifdef _MSC_VER
	#include <intrin.h>
	#ifdef _WIN64
		#define ATOMIC_INCREMENT( p ) _InterlockedExchangeAdd64( (__int64 volatile*)( p ), 1 )
		#define ATOMIC_DECREMENT( p ) _InterlockedExchangeAdd64( (__int64 volatile*)( p ), -1 )
		#define ATOMIC_COMPARE_AND_SWAP(p, o, n) (_InterlockedCompareExchange128( (__int64*)(p), (n)[1], (n)[0], (o) ) == 1)
	#else
		#define ATOMIC_INCREMENT( p ) _InterlockedExchangeAdd( (long volatile*)( p ), 1 )
		#define ATOMIC_DECREMENT( p ) _InterlockedExchangeAdd( (long volatile*)( p ), -1 )
		#define ATOMIC_COMPARE_AND_SWAP(p, o, n) (_InterlockedCompareExchange( (p), (n), (o) ) == (o))
	#endif
#elif __GNUC__
	#define ATOMIC_INCREMENT( p ) __atomic_add_fetch( p, 1, __ATOMIC_RELAXED )
	#define ATOMIC_DECREMENT( p ) __atomic_sub_fetch( p, 1, __ATOMIC_RELAXED )
	#ifdef __x86_64__
		// NOTE: compiler flag '-mcx16' is required, and may require linker flag '-latomic' in aarch64
		#define ATOMIC_COMPARE_AND_SWAP(p, o, n) __sync_bool_compare_and_swap( (p), (o), (n) )
	#else
		#define ATOMIC_COMPARE_AND_SWAP(p, o, n) __sync_bool_compare_and_swap( (p), (o), (n) )
	#endif
#endif

void atomic_slist_init( ATOMIC_SLIST_HEAD* head )
{
	assert( ((size_t)head) % sizeof(*head) == 0 );
	memset( head, 0, sizeof(*head) );
}



void atomic_slist_push( ATOMIC_SLIST_HEAD* head, ATOMIC_SLIST_NODE* node )
{
	ATOMIC_SLIST_HEAD o, n;
	n.next = node;
	do {
		o = *head;
		node->next = o.next;
		n.cnt = o.cnt + 1;
	} while( !ATOMIC_COMPARE_AND_SWAP( &head->alignment, o.alignment, n.alignment ) );
}



ATOMIC_SLIST_NODE* atomic_slist_pop( ATOMIC_SLIST_HEAD* head )
{
	ATOMIC_SLIST_HEAD o, n;
	do {
		o = *head;
		if( o.next == NULL )
			return NULL;
		n.next = o.next->next;
		n.cnt = o.cnt + 1;
	} while( !ATOMIC_COMPARE_AND_SWAP( &head->alignment, o.alignment, n.alignment ) );
	return o.next;
}