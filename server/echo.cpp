#include "stdafx.h"
#include <debug.h>
#include "iocp.h"

class CEchoPacket : public CNetPacket
{
private:
	SOCKET m_sck;
	BYTE m_buf[256];
	bool m_sending;

public:
	CEchoPacket( SOCKET sck )
	{
		m_sck = sck;
	}

	~CEchoPacket()
	{
		if( m_sck != INVALID_SOCKET )
			closesocket( m_sck );
	}

	void Receive()
	{
		m_sending = false;
		if( !BeginReceive( m_sck, m_buf, sizeof( m_buf ) ) )
			delete this;
	}

	void Send()
	{
		printf( "tcp echo server: %s\n", m_buf );
		m_sending = true;
		if( !BeginSend( m_sck, m_buf, sizeof( m_buf ) ) )
			delete this;
	}

	void OnIoComplete( ULONG_PTR key, DWORD cb )
	{
		if( m_sending )
			Receive();
		else
			Send();
	}

	void OnIoFailure( ULONG_PTR key, DWORD cb )
	{
		delete this;
	}
};


////////////////////////////////////////////////////////////////////////////////


class CEchoAcceptPacket : public CAcceptPacket
{
public:
	void OnAccept( SOCKET sck, ULONG addr, USHORT port )
	{
		_tprintf( _T( "new connection on echo socket\n" ) );

		iocp_bind_socket( sck, static_cast<ULONG_PTR>(sck) );
		CEchoPacket* pkt = new CEchoPacket( sck );
		if( pkt != NULL )
			pkt->Receive();
		else
			closesocket( sck );
	}
};


////////////////////////////////////////////////////////////////////////////////

static SOCKET g_listen = INVALID_SOCKET;


bool init_echo_listen_socket()
{
	g_listen = ::socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( g_listen == INVALID_SOCKET )
		return false;

	SOCKADDR_IN sa ={0};
	sa.sin_family = AF_INET;
	sa.sin_addr.S_un.S_addr = INADDR_ANY;
	sa.sin_port = htons( 9999 );

	int err = ::bind( g_listen, reinterpret_cast<sockaddr*>(&sa), sizeof( sa ) );
	if( err == SOCKET_ERROR )
		return false;

	err = ::listen( g_listen, 2 );
	if( err == SOCKET_ERROR )
		return false;

	if( !iocp_bind_socket( g_listen, static_cast<ULONG_PTR>(g_listen) ) )
		return false;

	CEchoAcceptPacket* pkt = new CEchoAcceptPacket();
	if( pkt == NULL )
		return false;

	if( !pkt->BeginAccept( g_listen ) )
	{
		delete pkt;
		return false;
	}

	return true;
}

void close_echo_listen_socket()
{
	// we should record all the accepted SOCKETs and close them here,
	// buf as this is only for demo we haven't do this
	::closesocket( g_listen );
	g_listen = INVALID_SOCKET;
}


////////////////////////////////////////////////////////////////////////////////
