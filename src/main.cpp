#include <stdio.h>
// #include "iocp.h"
#include "mempool.h"
#include <atomic>

int main( int argc, char** argv )
{
    std::atomic<__int128> t;
    printf("%d\n", t.is_lock_free());
    
    void* p = mem_pool_init( 28, 19 );
    void* a = mem_pool_alloc( p );
    mem_pool_free( p, a );
    mem_pool_uninit( p );
    /*
    iocp p;
    int n = p.start( 0 );
    printf( "%d threads created\n", n );
    getchar();
    p.stop();

*/
    return 0;
}