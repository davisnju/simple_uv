#pragma once
#include "tcpserver.h"
#include "../message_define/TestMessage.h"

class CTestGateWay :
	public TCPServer
{
public:
	CTestGateWay();
	virtual ~CTestGateWay();

protected:
	BEGIN_UV_BIND
		UV_BIND(CTestMsg::MSG_ID, CTestMsg)
	END_UV_BIND(TCPServer)

	BEGIN_UV_THREAD_BIND
	UV_THREAD_BIND(CTestMsg::MSG_ID, CTestMsg)
	END_UV_THREAD_BIND(TCPServer)

	int OnUvMessage(const CTestMsg &msg, TcpClientCtx *pClient);

	void OnUvThreadMessage(CTestMsg msg, unsigned int nSrcAddr);

	int m_nSessionID;

private:
	map<int, TcpClientCtx *> m_mapSession;
};

