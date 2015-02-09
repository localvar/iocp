#include "stdafx.h"

static HANDLE g_stop_event = NULL;

static unsigned __stdcall math_thread( void* )
{
	struct MATHCOMMAND
	{
		int op;
		int a;
		int b;
	} cmd;
	const char op[] ={'+', '-', '*', '/'};

	srand( GetTickCount() );

	SOCKET sck = ::socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( sck == INVALID_SOCKET )
		return 1;

	SOCKADDR_IN sa ={0};
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr( "127.0.0.1" );
	sa.sin_port = htons( 8888 );

	int err = connect( sck, reinterpret_cast<sockaddr*>(&sa), sizeof( sa ) );
	if( err == SOCKET_ERROR )
		return 0;

	while( WaitForSingleObject( g_stop_event, 1000 ) == WAIT_TIMEOUT )
	{
		cmd.a = rand();
		cmd.b = rand();
		cmd.op = op[rand() % 4];
		send( sck, reinterpret_cast<const char*>(&cmd), sizeof( cmd ), 0 );
		int result = 0;
		recv( sck, reinterpret_cast<char*>(&result), sizeof( result ), 0 );
		printf( "math: %d %c %d = %d\n", cmd.a, cmd.op, cmd.b, result );
	}

	closesocket( sck );
	return 0;
}


static unsigned __stdcall echo_thread( void* )
{
	SOCKET sck = ::socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( sck == INVALID_SOCKET )
		return 1;

	SOCKADDR_IN sa ={0};
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr( "127.0.0.1" );
	sa.sin_port = htons( 9999 );

	int err = connect( sck, reinterpret_cast<sockaddr*>(&sa), sizeof( sa ) );
	if( err == SOCKET_ERROR )
		return 0;

	int i = 0;
	while( WaitForSingleObject( g_stop_event, 1000 ) == WAIT_TIMEOUT )
	{
		char buf[256];
		sprintf( buf, "tcp echo %d", i++ );
		send( sck, buf, strlen( buf ) + 1, 0 );
		buf[0] = 0;
		int len = recv( sck, buf, sizeof( buf ), 0 );
		puts( buf );
	}

	closesocket( sck );
	return 0;
}


static unsigned __stdcall udp_thread( void* )
{
	SOCKET sck = ::socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if( sck == INVALID_SOCKET )
		return 1;

	SOCKADDR_IN sa ={0};
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr( "127.0.0.1" );
	sa.sin_port = htons( 7777 );

	int err = connect( sck, reinterpret_cast<sockaddr*>(&sa), sizeof( sa ) );
	if( err == SOCKET_ERROR )
		return 0;

	int i = 0;
	while( WaitForSingleObject( g_stop_event, 1000 ) == WAIT_TIMEOUT )
	{
		char buf[256];
		sprintf( buf, "udp echo %d", i++ );
		send( sck, buf, strlen( buf ) + 1, 0 );
		buf[0] = 0;
		int len = recv( sck, buf, sizeof( buf ), 0 );
		puts( buf );
	}

	closesocket( sck );
	return 0;
}

int _tmain( int argc, _TCHAR* argv[] )
{
	WSADATA data;
	WSAStartup( MAKEWORD( 2, 2 ), &data );

	g_stop_event = CreateEvent( NULL, TRUE, FALSE, NULL );

	uintptr_t h = _beginthreadex( NULL, 0, math_thread, NULL, 0, NULL );
	::CloseHandle( reinterpret_cast<HANDLE>(h) );

	h = _beginthreadex( NULL, 0, echo_thread, NULL, 0, NULL );
	::CloseHandle( reinterpret_cast<HANDLE>(h) );

	h = _beginthreadex( NULL, 0, udp_thread, NULL, 0, NULL );
	::CloseHandle( reinterpret_cast<HANDLE>(h) );

	_tprintf( _T( "press any key to exit...\n" ) );
	_gettch();

	SetEvent( g_stop_event );
	Sleep( 1000 );
	CloseHandle( g_stop_event );

	WSACleanup();
	return 0;
}

