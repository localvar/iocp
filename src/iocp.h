#include <stdint.h>
#include <pthread.h>

#pragma once


class iocp
{
private:
    pthread_t* m_threads;
    int m_numThreads;
    int m_fd;

    static void* thread_stub( void * );
    void thread();

public:
    iocp();
    ~iocp();
    bool bind( int fd, void* arg );
    void unbind( int fd );
    int start( int numThreads );
    void stop();
};


class CIoPacket
{
public:
    CIoPacket();
    virtual ~CIoPacket();
    virtual void OnIoComplete( void* key, uintptr_t cb ) = 0;
    virtual void OnIoFailure( void* key, uintptr_t cb ) = 0;

private:
    CIoPacket( const CIoPacket& ) = delete;
    CIoPacket& operator=( const CIoPacket& ) = delete;
};
