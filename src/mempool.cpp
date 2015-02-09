#include "mempool.h"
#include <assert.h>
#include <memory.h>
#include <stdlib.h>
#include "atomic.h"



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

typedef struct
{
	ATOMIC_SLIST_HEAD lsthdr;	// free list
	size_t size;				// size of each block
	size_t count;				// count of blocks
	size_t used;				// blocks already alloced
	size_t dummy;				// for alignment
	char buf[0];				// the buffer
} pool_t;



void* mem_pool_init( size_t size, size_t count )
{
	if( size < sizeof( ATOMIC_SLIST_NODE ) )
		size = sizeof( ATOMIC_SLIST_NODE );

	char* addr = (char*)malloc( sizeof( pool_t ) + size * count );
	if( addr == NULL )
		return NULL;

	pool_t* pool = (pool_t*)addr;
	atomic_slist_init( &pool->lsthdr );
	pool->size = size;
	pool->count = count;
	pool->used = 0;

	addr = pool->buf + size * count;
	for( size_t i = 0; i < count; ++i ) {
		addr -= size;
		atomic_slist_push( &pool->lsthdr, (ATOMIC_SLIST_NODE*)addr );
	}

	return pool;
}



void mem_pool_uninit( void* p )
{
	free( p );
}


void* mem_pool_alloc( void* handle )
{
	pool_t* pool = (pool_t*)handle;
	ATOMIC_SLIST_NODE* node = atomic_slist_pop( &pool->lsthdr );
	if( node != NULL )
		ATOMIC_INCREMENT( &pool->used );
	return node;
}



void mem_pool_free( void* handle, void* mem )
{
	pool_t* pool = (pool_t*)handle;

	if( mem == NULL )
		return;

	ptrdiff_t offset = (char*)mem - pool->buf;
	if( offset < 0 || offset % pool->size != 0 )
		return;

	if( offset / pool->size >= pool->size )
		return;

	ATOMIC_DECREMENT( &pool->used );
	atomic_slist_push( &pool->lsthdr, (ATOMIC_SLIST_NODE*)mem );
}
