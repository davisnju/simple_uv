#include "TestGateWay.h"
#include "UVThreadMng.h"


CTestGateWay::CTestGateWay()
	: m_nSessionID(0)
{
}


CTestGateWay::~CTestGateWay()
{
}

int CTestGateWay::OnUvMessage(const CTestMsg &msg, TcpClientCtx *pClient)
{
	return CUVThreadMng::GetInstance()->SendUvMessage(msg, msg.MSG_ID, SERVER_TEST_CENTER_TYPE);
}
