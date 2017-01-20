#include "TestServerCenter.h"
#include "../message_define/TestMessage.h"
#include "UVThreadMng.h"


CTestServerCenter::CTestServerCenter(void)
	: CUVThread(SERVER_TEST_CENTER_TYPE)
{
}


CTestServerCenter::~CTestServerCenter(void)
{
}

void CTestServerCenter::OnUvThreadMessage( CTestMsg msg, unsigned int nSrcAddr )
{
	CUVThreadMng::GetInstance()->SendUvMessage2Handle(msg, msg.MSG_ID, m_nThreadType);
}
