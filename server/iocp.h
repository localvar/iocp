#pragma once


////////////////////////////////////////////////////////////////////////////////


#pragma warning( push )
#pragma warning( disable: 4200 )

struct IOCP_COMMAND
{
	DWORD dwCmdId;
	DWORD cbCmd;
	char Command[0];
};


struct IOCP_REPLY
{
	DWORD dwError;
	DWORD cbReply;
	char Reply[0];
};

#pragma warning( pop )


////////////////////////////////////////////////////////////////////////////////


bool iocp_bind_device( HANDLE dev, ULONG_PTR key );
bool iocp_bind_socket( SOCKET sck, ULONG_PTR key );
bool iocp_create();
bool iocp_post_status( DWORD cb, ULONG_PTR key, LPOVERLAPPED pol );
void iocp_destroy();


////////////////////////////////////////////////////////////////////////////////
// The base of all I/O packets

class __declspec(novtable) CIoPacket : public OVERLAPPED
{
public:
	CIoPacket();
	void Reset();
	virtual ~CIoPacket();
	virtual void OnIoComplete( ULONG_PTR key, DWORD cb ) = 0;
	virtual void OnIoFailure( ULONG_PTR key, DWORD cb ) = 0;

private:
	CIoPacket( const CIoPacket& );
	CIoPacket& operator=(const CIoPacket&);
};


////////////////////////////////////////////////////////////////////////////////
// CAcceptPacket

class __declspec( novtable ) CAcceptPacket : public CIoPacket
{
protected:
	SOCKET m_sck;
	BYTE m_buf[(sizeof( SOCKADDR_IN ) + 16) * 2];

public:
	virtual void OnAccept( SOCKET sck, ULONG addr, USHORT port ) = 0;

public:
	CAcceptPacket();
	~CAcceptPacket();
	bool BeginAccept( SOCKET sck );
	void OnIoComplete( ULONG_PTR key, DWORD cb );
	void OnIoFailure( ULONG_PTR key, DWORD cb );
};


////////////////////////////////////////////////////////////////////////////////
// CNetPacket

class __declspec(novtable) CNetPacket : public CIoPacket
{
protected:
	bool BeginReceive( SOCKET sck, LPVOID buf, DWORD cb );
	bool BeginSend( SOCKET sck, LPVOID buf, DWORD cb );
	bool BeginReceiveFrom( SOCKET sck, LPVOID buf, DWORD cb, SOCKADDR_IN* from );
	bool BeginSendTo( SOCKET sck, LPVOID buf, DWORD cb, const SOCKADDR_IN* to );
};


////////////////////////////////////////////////////////////////////////////////
