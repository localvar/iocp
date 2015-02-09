#include "stdafx.h"
#include <debug.h>
#include "iocp.h"


class CUdpPacket : public CNetPacket
{
private:
	SOCKET m_sck;
	BYTE m_buf[256];
	bool m_sending;
	SOCKADDR_IN m_peer;

public:
	CUdpPacket( SOCKET sck )
	{
		m_sck = sck;
	}

	~CUdpPacket()
	{
		if( m_sck != INVALID_SOCKET )
			closesocket( m_sck );
	}

	void Receive()
	{
		m_sending = false;
		if( !BeginReceiveFrom( m_sck, m_buf, sizeof( m_buf ), &m_peer ) )
			delete this;
	}

	void Send()
	{
		m_sending = true;
		if( !BeginSendTo( m_sck, m_buf, sizeof( m_buf ), &m_peer ) )
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

static SOCKET g_udp = INVALID_SOCKET;


bool init_udp_socket()
{
	g_udp = ::socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if( g_udp == INVALID_SOCKET )
		return false;

	SOCKADDR_IN sa ={0};
	sa.sin_family = AF_INET;
	sa.sin_addr.S_un.S_addr = INADDR_ANY;
	sa.sin_port = htons( 7777 );

	int err = ::bind( g_udp, reinterpret_cast<sockaddr*>(&sa), sizeof( sa ) );
	if( err == SOCKET_ERROR )
		return false;

	if( !iocp_bind_socket( g_udp, static_cast<ULONG_PTR>(g_udp) ) )
		return false;

	CUdpPacket* pkt = new CUdpPacket( g_udp );
	if( pkt == NULL )
		return false;

	pkt->Receive();
	return true;
}

void close_udp_socket()
{
	// we should record all the accepted SOCKETs and close them here,
	// buf as this is only for demo we haven't do this
	::closesocket( g_udp );
	g_udp = INVALID_SOCKET;
}


////////////////////////////////////////////////////////////////////////////////
