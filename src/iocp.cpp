#include <sys/epoll.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#include "iocp.h"



iocp::iocp() : m_fd( -1 ), m_numThreads( 0 ), m_threads( nullptr )
{
    m_fd = epoll_create( 1 );
}



iocp::~iocp()
{
    stop();
    if( m_fd != -1 )
    {
        close( m_fd );
        m_fd = -1;
    }
}



bool iocp::bind( int fd, void* arg )
{
    epoll_event event = { 0 };
    event.events = EPOLLIN | EPOLLOUT;
    event.data.ptr = arg;
    return epoll_ctl( m_fd, EPOLL_CTL_ADD, fd, &event ) == 0;
}



void iocp::unbind( int fd )
{
    epoll_event event = { 0 };
    epoll_ctl( m_fd, EPOLL_CTL_DEL, fd, &event );
}



int iocp::start( int numThreads )
{
    if( numThreads <= 0 )
    {
        numThreads = get_nprocs_conf() * 2 + 2;
    }

    m_threads = reinterpret_cast<pthread_t*>( calloc( numThreads, sizeof(pthread_t) ) );
    if( m_threads == nullptr )
    {
        return false;
    }

    for( int i = 0; i < numThreads; ++i )
    {
        if( pthread_create( m_threads + i, nullptr, thread_stub, this ) != 0 )
        {
            break;
        }
        ++m_numThreads;
    }

    return m_numThreads;
}



void iocp::stop()
{
    for( int i = 0; i < m_numThreads; i++ )
    {
        pthread_cancel( m_threads[i] );
    }

    for( int i = 0; i < m_numThreads; i++ )
    {
        pthread_join( m_threads[i], nullptr );
    }

    free( m_threads );
    m_threads = nullptr;
    m_numThreads = 0;
}



void* iocp::thread_stub( void* arg )
{
    reinterpret_cast<iocp*>( arg )->thread();
    return nullptr;
}



void iocp::thread()
{
    while( true )
    {
        epoll_event event = { 0 };
        int r = epoll_wait( m_fd, &event, 1, -1 );
        if( r == -1 )
        {

        }
    }
}



CIoPacket::CIoPacket()
{

}



CIoPacket::~CIoPacket()
{

}
