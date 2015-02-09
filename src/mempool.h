#pragma once

#include <stddef.h>

void* mem_pool_init( size_t size, size_t count );
void mem_pool_uninit( void* p );
void* mem_pool_alloc( void* handle );
void mem_pool_free( void* handle,  void* mem );
