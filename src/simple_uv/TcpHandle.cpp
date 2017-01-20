// #include "stdafx.h"
#include "TcpHandle.h"
#include "thread_uv.h"
#include "BaseMsgDefine.h"
#include <assert.h>
#include "uv_msg_framing.h"
#include "UVThreadMng.h"


CTcpHandle::CTcpHandle()
	: m_bIsClosed(true)
	, m_bIsUserAskForClosed(false)
	, m_cPacketHead(SERVER_PACKET_HEAD)
	, m_cPacketTail(SERVER_PACKET_TAIL)
{
	m_strErrMsg = new std::string;
	int iret = uv_loop_init(&m_loop);
	if (iret) {
		*m_strErrMsg = GetUVError(iret);
		fprintf(stdout, "init loop error: %s\n", m_strErrMsg->c_str());
	}
	iret = uv_mutex_init(&m_mutexClients);
	if (iret) {
		*m_strErrMsg = GetUVError(iret);
		fprintf(stdout, "uv_mutex_init error: %s\n", m_strErrMsg->c_str());
	}
}


CTcpHandle::~CTcpHandle(void)
{
	uv_mutex_destroy(&m_mutexClients);
	uv_loop_close(&m_loop);
}

void  CTcpHandle::Close()
{
	if (m_bIsClosed) {
		return;
	}
}

bool  CTcpHandle::SetNoDelay(bool enable)
{
	int iret = uv_tcp_nodelay(&m_tcpHandle, enable ? 1 : 0);
	if (iret) {
		*m_strErrMsg = GetUVError(iret);
		//        // LOGE(errmsg_);
		return false;
	}
	return true;
}

bool  CTcpHandle::SetKeepAlive(int enable, unsigned int delay)
{
	int iret = uv_tcp_keepalive(&m_tcpHandle, enable, delay);
	if (iret) {
		*m_strErrMsg = GetUVError(iret);
		// // LOGI(errmsg_);
		return false;
	}
	return true;
}

bool CTcpHandle::init()
{
	if (!m_bIsClosed) {
		return true;
	}
	int iret = uv_async_init(&m_loop, &m_asyncHandle, AsyncCloseCB);
	if (iret) {
		*m_strErrMsg = GetUVError(iret);
		//        // LOGE(errmsg_);
		return false;
	}
	m_asyncHandle.data = this;

	uv_async_init(&m_loop, &m_asyncHandleForRecvMsg, AsyncRecvMsg);
	uv_msg_thread_2_handle *pData = new uv_msg_thread_2_handle;
	pData->m_pClass = this;
	pData->m_pData = nullptr;
	m_asyncHandleForRecvMsg.data = pData;

	CUVThreadMng::GetInstance()->RegistHandle(&m_asyncHandleForRecvMsg);

	iret = uv_tcp_init(&m_loop, &m_tcpHandle);
	if (iret) {
		*m_strErrMsg = GetUVError(iret);
		//        // LOGE(errmsg_);
		return false;
	}
	m_tcpHandle.data = this;
	
	m_bIsClosed = false;
	return true;
}

void CTcpHandle::AsyncCloseCB( uv_async_t* handle )
{
	CTcpHandle* theclass = (CTcpHandle*)handle->data;
	if (theclass->m_bIsUserAskForClosed) {
		theclass->closeinl();
		return;
	}
	//check data to send
	theclass->send_inl(NULL);
}

void  CTcpHandle::send_inl( uv_write_t* req /*= NULL*/ )
{

}

int CTcpHandle::ParsePacket(const NetPacket& packet, const unsigned char* buf, TcpClientCtx *pClient)
{
	return -1;
}

void CTcpHandle::closeinl()
{
	
}

bool CTcpHandle::run(int status /*= UV_RUN_DEFAULT*/)
{
	int iret = uv_run(&m_loop, (uv_run_mode)status);
	if (iret) {
		*m_strErrMsg = GetUVError(iret);
		
		this->OnExit();
		return false;
	}
	this->OnExit();
	return true;
}

void CTcpHandle::AsyncRecvMsg( uv_async_t* handle )
{
	uv_msg_thread_2_handle* pData = (uv_msg_thread_2_handle*)handle->data;

	if (!pData->m_pData || !pData->m_pClass)
	{
		return;
	}

	CTcpHandle* pClass = (CTcpHandle*)pData->m_pClass;
	NodeMsg *req = (NodeMsg *)pData->m_pData;

	pClass->OnDispatchMsg(req->m_nMsgType, req->m_pData, req->m_nSrcAddr);

	delete req;
	req = nullptr;
	pData->m_pData = nullptr;
}

void CTcpHandle::OnExit()
{
	CUVThreadMng::GetInstance()->UnRegistHandle();
}

void GetPacket(const NetPacket& packethead, const unsigned char* packetdata, void* userdata)
{
	fprintf(stdout, "Get control packet type %d\n", packethead.type);
	assert(userdata);
	TcpClientCtx* theclass = (TcpClientCtx*)userdata;
	CTcpHandle* parent = (CTcpHandle*)theclass->parent_server;
	parent->ParsePacket(packethead, packetdata, theclass);
	return;
}
