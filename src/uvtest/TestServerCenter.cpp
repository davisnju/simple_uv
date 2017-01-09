#include "stdafx.h"
#include "TestServerCenter.h"
#include "../message_define/TestMessage.h"


CTestServerCenter::CTestServerCenter(void)
	: CUVThread(SERVER_TEST_CENTER_TYPE)
{
}


CTestServerCenter::~CTestServerCenter(void)
{
}

void CTestServerCenter::OnUvThreadMessage( CTestMsg msg, unsigned int nSrcAddr )
{
	msg.m_nSessionID++;
	// this->SendUvMessage(msg, msg.MSG_ID, SERVER_TEST_CENTER_TYPE);
}
