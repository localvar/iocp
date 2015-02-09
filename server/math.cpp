#include "stdafx.h"
#include <debug.h>
#include "iocp.h"


////////////////////////////////////////////////////////////////////////////////

struct MATHCOMMAND
{
	int op;
	int a;
	int b;
};

class CMathPacket : public CNetPacket
{
private:
	SOCKET m_sck;
	MATHCOMMAND m_cmd;
	int m_result;
	bool m_sending;

public:
	CMathPacket( SOCKET sck )
	{
		m_sck = sck;
	}

	~CMathPacket()
	{
		if( m_sck != INVALID_SOCKET )
			closesocket( m_sck );
	}

	void Receive()
	{
		m_sending = false;
		if( !BeginReceive( m_sck, &m_cmd, sizeof( m_cmd ) ) )
			delete this;
	}

	void Send()
	{
		m_sending = true;
		if( !BeginSend( m_sck, &m_result, sizeof(m_result) ) )
			delete this;
	}

	void OnIoComplete( ULONG_PTR key, DWORD cb )
	{
		if( m_sending )
		{
			Receive();
			return;
		}

		m_result = 0;
		switch( m_cmd.op )
		{
		case '+':
			m_result = m_cmd.a + m_cmd.b;
			break;
		case '-':
			m_result = m_cmd.a - m_cmd.b;
			break;
		case '*':
			m_result = m_cmd.a * m_cmd.b;
			break;
		case '/':
			if( m_cmd.b != 0 )
				m_result = m_cmd.a / m_cmd.b;
			break;
		}
		printf( "math server: %d %c %d = %d\n", m_cmd.a, m_cmd.op, m_cmd.b, m_result );
		Send();
	}

	void OnIoFailure( ULONG_PTR key, DWORD cb )
	{
		delete this;
	}
};


////////////////////////////////////////////////////////////////////////////////


class CMathAcceptPacket : public CAcceptPacket
{
public:
	void OnAccept( SOCKET sck, ULONG addr, USHORT port )
	{
		_tprintf( _T( "new connection on math socket\n" ) );

		iocp_bind_socket( sck, static_cast<ULONG_PTR>(sck) );
		CMathPacket* pkt = new CMathPacket( sck );
		if( pkt != NULL )
			pkt->Receive();
		else
			closesocket( sck );
	}
};


////////////////////////////////////////////////////////////////////////////////

static SOCKET g_listen = INVALID_SOCKET;


bool init_math_listen_socket()
{
	g_listen = ::socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( g_listen == INVALID_SOCKET )
		return false;

	SOCKADDR_IN sa ={0};
	sa.sin_family = AF_INET;
	sa.sin_addr.S_un.S_addr = INADDR_ANY;
	sa.sin_port = htons( 8888 );

	int err = ::bind( g_listen, reinterpret_cast<sockaddr*>(&sa), sizeof( sa ) );
	if( err == SOCKET_ERROR )
		return false;

	err = ::listen( g_listen, 2 );
	if( err == SOCKET_ERROR )
		return false;

	if( !iocp_bind_socket( g_listen, static_cast<ULONG_PTR>(g_listen) ) )
		return false;

	CMathAcceptPacket* pkt = new CMathAcceptPacket();
	if( pkt == NULL )
		return false;

	if( !pkt->BeginAccept( g_listen ) )
	{
		delete pkt;
		return false;
	}

	return true;
}

void close_math_listen_socket()
{
	// we should record all the accepted SOCKETs and close them here,
	// buf as this is only for demo we haven't do this
	::closesocket( g_listen );
	g_listen = INVALID_SOCKET;
}


////////////////////////////////////////////////////////////////////////////////