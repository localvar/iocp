#include "stdafx.h"


int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA data;
	WSAStartup( MAKEWORD( 2, 2 ), &data );

	SOCKET sck = ::socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if( sck == INVALID_SOCKET )
		return 1;

	SOCKADDR_IN sa ={0};
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr("127.0.0.1");
	sa.sin_port = htons( 7777 );

	int err = connect( sck, reinterpret_cast<sockaddr*>(&sa), sizeof( sa ) );
	if( err == SOCKET_ERROR )
		return false;

	for( int i = 0;; ++i )
	{
		char buf[256];
		sprintf( buf, "count %d", i );
		send( sck, buf, strlen( buf ) + 1, 0 );
		buf[0] = 0;
		int len = recv( sck, buf, sizeof(buf), 0 );
		puts( buf );
		Sleep( 1000 );
	}

	closesocket( sck );
	WSACleanup();
	return 0;
}

