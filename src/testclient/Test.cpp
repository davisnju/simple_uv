#include "Test.h"


CTest::CTest()
{
}


CTest::~CTest()
{
}

int CTest::OnUvMessage(const CTestMsg &msg, TcpClientCtx *pClient)
{
	return this->SendUvMessage(msg, msg.MSG_ID);
}
