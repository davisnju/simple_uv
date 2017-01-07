#include "stdafx.h"
#include "TestServer.h"


CTestServer::CTestServer()
{
}


CTestServer::~CTestServer()
{
}

int CTestServer::OnUvMessage(const CTestMsg &msg, TcpClientCtx *pClient)
{
	return this->SendUvMessage(msg, msg.MSG_ID, pClient);
}
