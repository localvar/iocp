#include "stdafx.h"
#include <debug.h>
#include "iocp.h"


bool init_math_listen_socket();
void close_math_listen_socket();

bool init_echo_listen_socket();
void close_echo_listen_socket();

bool init_udp_socket();
void close_udp_socket();


int _tmain( int argc, _TCHAR* argv[] )
{
	WSADATA data;
	DWORD err = WSAStartup( MAKEWORD( 2, 2 ), &data );

	iocp_create();

	init_math_listen_socket();
	init_echo_listen_socket();
	init_udp_socket();

	_tprintf( _T( "press any key to exit...\n" ) );
	_gettch();

	close_udp_socket();
	close_echo_listen_socket();
	close_math_listen_socket();

	Sleep( 1000 );
	iocp_destroy();

	WSACleanup();
	return 0;
}

