#include "stdafx.h"
#include <debug.h>
#include "iocp.h"

////////////////////////////////////////////////////////////////////////////////
// global variables

static HANDLE g_iocp = NULL;
static volatile long g_num_thread = 0;		// number of worker thread
static volatile long g_num_packet = 0;		// number of i/o packets


////////////////////////////////////////////////////////////////////////////////


static DWORD iocp_thread()
{
	DWORD ret = ERROR_SUCCESS;

	while( true )
	{
		DWORD cb = 0;
		ULONG_PTR key = 0;
		LPOVERLAPPED pol = NULL;

		if( ::GetQueuedCompletionStatus( g_iocp, &cb, &key, &pol, INFINITE ) )
		{
			if( key == 0 )	//special packet received, should exit
			{
				break;
			}
			else	//everything is ok
			{
				static_cast<CIoPacket*>(pol)->OnIoComplete( key, cb );
			}
		}
		else
		{
			if( pol == NULL )	//caught a fatal error
			{
				ret = GetLastError();
				break;
			}
			else	//an io failure was encountered
			{
				static_cast<CIoPacket*>(pol)->OnIoFailure( key, cb );
			}
		}
	}

	return ret;
}


//------------------------------------------------------------------------------


static unsigned __stdcall iocp_thread_stub( void* )
{
	_InterlockedIncrement( &g_num_thread );
	DWORD err = iocp_thread();
	_InterlockedDecrement( &g_num_thread );
	return err;
}


//------------------------------------------------------------------------------
// Create an iocp thread

static bool create_iocp_thread()
{
	uintptr_t handle = ::_beginthreadex(
		NULL,
		0,
		iocp_thread_stub,
		NULL,
		0,
		NULL
		);

	ASSERT( handle != 0 );

	if( handle != 0 )
	{
		::CloseHandle( reinterpret_cast<HANDLE>(handle) );
		return true;
	}

	return false;
}


////////////////////////////////////////////////////////////////////////////////
// Iocp Management

bool iocp_bind_device( HANDLE dev, ULONG_PTR key )
{
	HANDLE iocp = ::CreateIoCompletionPort( dev, g_iocp, key, 0 );
	ASSERT( iocp == g_iocp );
	return iocp == g_iocp;
}


//------------------------------------------------------------------------------


bool iocp_bind_socket( SOCKET sck, ULONG_PTR key )
{
	return iocp_bind_device( reinterpret_cast<HANDLE>(sck), key );
}



////////////////////////////////////////////////////////////////////////////////

static DWORD get_num_cpu()
{
	SYSTEM_INFO si;
	GetSystemInfo( &si );
	return si.dwNumberOfProcessors;
}

//------------------------------------------------------------------------------

bool iocp_create()
{
	DWORD numcpu = get_num_cpu();
	DWORD numthread = numcpu + 2;

	// create the IOCP
	g_iocp = ::CreateIoCompletionPort(
		INVALID_HANDLE_VALUE,
		NULL,
		NULL,
		numcpu
		);
	if( g_iocp == NULL )
		return false;

	// create iocp thread
	for( DWORD i = 0; i < numthread; ++i )
		if( !create_iocp_thread() )
			return false;

	return true;
}


//------------------------------------------------------------------------------


bool iocp_post_status( DWORD cb, ULONG_PTR key, LPOVERLAPPED pol )
{
	bool ret = !!::PostQueuedCompletionStatus( g_iocp, cb, key, pol );
	ASSERT( ret );
	return ret;
}


//------------------------------------------------------------------------------


void iocp_destroy()
{
	if( g_iocp != NULL )
	{
		// tell worker threads to stop
		long numthread = g_num_thread;
		for( long i = 0; i < numthread; ++i )
			iocp_post_status( 0, NULL, NULL );

		// and wait until all of them stops
		while( g_num_thread > 0 )
			::Sleep( 100 );

		// Close the handle.
		::CloseHandle( g_iocp );
		g_iocp = NULL;
	}
}


////////////////////////////////////////////////////////////////////////////////
// ctor & dtor of CIoPacket

CIoPacket::CIoPacket()
{
	_InterlockedIncrement( &g_num_packet );
	Reset();
}

void CIoPacket::Reset()
{
	// Initialize the member of OVERLAPPED structure
	Internal = InternalHigh = Offset = OffsetHigh = 0;
	hEvent = NULL;
}

CIoPacket::~CIoPacket()
{
	_InterlockedDecrement( &g_num_packet );
}


////////////////////////////////////////////////////////////////////////////////
// CAcceptPacket


CAcceptPacket::CAcceptPacket()
	: m_sck( INVALID_SOCKET )
{
}


//------------------------------------------------------------------------------


CAcceptPacket::~CAcceptPacket()
{
	if( m_sck != INVALID_SOCKET )
	{
		::closesocket( m_sck );
		m_sck = INVALID_SOCKET;
	}
}


//------------------------------------------------------------------------------


bool CAcceptPacket::BeginAccept( SOCKET scklisten )
{
	m_sck = ::socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( m_sck == INVALID_SOCKET )
		return false;

	// set send/recv buffer size to zero to improve performance
	int zero = 0;
	const char* val = reinterpret_cast<char*>(&zero);
	::setsockopt( m_sck, SOL_SOCKET, SO_SNDBUF, val, sizeof( zero ) );
	::setsockopt( m_sck, SOL_SOCKET, SO_RCVBUF, val, sizeof( zero ) );

	const DWORD len = sizeof( SOCKADDR_IN ) + 16;
	DWORD dw;
	if( !::AcceptEx( scklisten, m_sck, m_buf, 0, len, len, &dw, this ) )
		if( WSAGetLastError() != ERROR_IO_PENDING )
			return false;

	return true;
}


//------------------------------------------------------------------------------


void CAcceptPacket::OnIoComplete( ULONG_PTR key, DWORD cb )
{
	SOCKET scklisten = static_cast<SOCKET>(key);

	// update socket information
	int opt = SO_UPDATE_ACCEPT_CONTEXT;
	const char* val = reinterpret_cast<char*>(&scklisten);
	::setsockopt( m_sck, SOL_SOCKET, opt, val, sizeof( SOCKET ) );

	// retrieve peer address and port
	const DWORD len = sizeof( SOCKADDR_IN ) + 16;
	LPSOCKADDR plocal, prmt;
	int local, rmt;
	::GetAcceptExSockaddrs( m_buf, 0, len, len, &plocal, &local, &prmt, &rmt );

	LPSOCKADDR_IN psa = reinterpret_cast<LPSOCKADDR_IN>(prmt);
	OnAccept( m_sck, psa->sin_addr.s_addr, psa->sin_port );

	// reset to reuse
	Reset();
	m_sck = INVALID_SOCKET;

	if( !BeginAccept( scklisten ) )
		delete this;
}


//------------------------------------------------------------------------------


void CAcceptPacket::OnIoFailure( ULONG_PTR key, DWORD cb )
{
	key, cb;
	delete this;
}


////////////////////////////////////////////////////////////////////////////////
// CNetPacket

bool CNetPacket::BeginReceive( SOCKET sck, LPVOID buf, DWORD cb )
{
	ASSERT( sck != INVALID_SOCKET );
	ASSERT( buf != NULL && cb > 0 );

	DWORD flag = 0, bytes = 0;
	LPOVERLAPPED pol = static_cast<LPOVERLAPPED>(this);

	// prepare io buffer
	WSABUF wsabuf;
	wsabuf.buf = reinterpret_cast<char*>(buf);
	wsabuf.len = static_cast<u_long>(cb);

	// receive
	if( ::WSARecv( sck, &wsabuf, 1, &bytes, &flag, pol, NULL ) != 0 )
		if( WSAGetLastError() != WSA_IO_PENDING )
			return false;

	return true;
}


//------------------------------------------------------------------------------


bool CNetPacket::BeginSend( SOCKET sck, LPVOID buf, DWORD cb )
{
	ASSERT( buf != NULL && cb > 0 );

	DWORD bytes = 0;
	LPOVERLAPPED pol = static_cast<LPOVERLAPPED>(this);

	// prepare io buffer
	WSABUF wsabuf;
	wsabuf.buf = reinterpret_cast<char*>(buf);
	wsabuf.len = static_cast<u_long>(cb);

	// send!
	if( ::WSASend( sck, &wsabuf, 1, &bytes, 0, pol, NULL ) != 0 )
		if( WSAGetLastError() != WSA_IO_PENDING )
			return false;

	return true;
}


//------------------------------------------------------------------------------


bool CNetPacket::BeginReceiveFrom( SOCKET sck, LPVOID buf, DWORD cb, SOCKADDR_IN* from )
{
	ASSERT( sck != INVALID_SOCKET );
	ASSERT( buf != NULL && cb > 0 );

	DWORD flag = 0, bytes = 0;
	LPOVERLAPPED pol = static_cast<LPOVERLAPPED>(this);

	// prepare io buffer
	WSABUF wsabuf;
	wsabuf.buf = reinterpret_cast<char*>(buf);
	wsabuf.len = static_cast<u_long>(cb);

	// receive
	int fl = sizeof( *from );
	sockaddr* f = reinterpret_cast<sockaddr*>(from);
	if( ::WSARecvFrom( sck, &wsabuf, 1, &bytes, &flag, f, &fl, pol, NULL ) != 0 )
		if( WSAGetLastError() != WSA_IO_PENDING )
			return false;

	return true;
}


//------------------------------------------------------------------------------


bool CNetPacket::BeginSendTo( SOCKET sck, LPVOID buf, DWORD cb, const SOCKADDR_IN* to )
{
	ASSERT( buf != NULL && cb > 0 );

	DWORD bytes = 0;
	LPOVERLAPPED pol = static_cast<LPOVERLAPPED>(this);

	// prepare io buffer
	WSABUF wsabuf;
	wsabuf.buf = reinterpret_cast<char*>(buf);
	wsabuf.len = static_cast<u_long>(cb);

	// send!
	const sockaddr* t = reinterpret_cast<const sockaddr*>(to);
	if( ::WSASendTo( sck, &wsabuf, 1, &bytes, 0, t, sizeof(*to), pol, NULL ) != 0 )
		if( WSAGetLastError() != WSA_IO_PENDING )
			return false;

	return true;
}


////////////////////////////////////////////////////////////////////////////////