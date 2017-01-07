#pragma once
#include "tcpserver.h"
#include "../message_define/TestMessage.h"

class CTestServer :
	public TCPServer
{
public:
	CTestServer();
	virtual ~CTestServer();

protected:
	BEGIN_UV_BIND
		UV_BIND(CTestMsg::MSG_ID, CTestMsg)
	END_UV_BIND(TCPServer)

	int OnUvMessage(const CTestMsg &msg, TcpClientCtx *pClient);
};

