#include "stdafx.h"
#include "TcpHandle.h"
#include "thread_uv.h"
#include "BaseMsgDefine.h"
#include <assert.h>


CTcpHandle::CTcpHandle(char packhead, char packtail)
	: isclosed_(true)
	, isuseraskforclosed_(false)
	, packet_head(packhead)
	, packet_tail(packtail)
{
	int iret = uv_loop_init(&loop_);
	if (iret) {
		errmsg_ = GetUVError(iret);
		fprintf(stdout, "init loop error: %s\n", errmsg_.c_str());
	}
	iret = uv_mutex_init(&mutex_clients_);
	if (iret) {
		errmsg_ = GetUVError(iret);
		fprintf(stdout, "uv_mutex_init error: %s\n", errmsg_.c_str());
	}
}


CTcpHandle::~CTcpHandle(void)
{
	uv_mutex_destroy(&mutex_clients_);
	uv_loop_close(&loop_);
}

void  CTcpHandle::Close()
{
	if (isclosed_) {
		return;
	}
}

bool  CTcpHandle::SetNoDelay(bool enable)
{
	int iret = uv_tcp_nodelay(&tcp_handle_, enable ? 1 : 0);
	if (iret) {
		errmsg_ = GetUVError(iret);
		//        // LOGE(errmsg_);
		return false;
	}
	return true;
}

bool  CTcpHandle::SetKeepAlive(int enable, unsigned int delay)
{
	int iret = uv_tcp_keepalive(&tcp_handle_, enable, delay);
	if (iret) {
		errmsg_ = GetUVError(iret);
		// // LOGI(errmsg_);
		return false;
	}
	return true;
}

bool CTcpHandle::init()
{
	if (!isclosed_) {
		return true;
	}
	int iret = uv_async_init(&loop_, &async_handle_, AsyncCloseCB);
	if (iret) {
		errmsg_ = GetUVError(iret);
		//        // LOGE(errmsg_);
		return false;
	}
	async_handle_.data = this;

	iret = uv_tcp_init(&loop_, &tcp_handle_);
	if (iret) {
		errmsg_ = GetUVError(iret);
		//        // LOGE(errmsg_);
		return false;
	}
	tcp_handle_.data = this;
	
	isclosed_ = false;
	return true;
}

void CTcpHandle::AsyncCloseCB( uv_async_t* handle )
{
	CTcpHandle* theclass = (CTcpHandle*)handle->data;
	if (theclass->isuseraskforclosed_) {
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
	int iret = uv_run(&loop_, (uv_run_mode)status);
	if (iret) {
		errmsg_ = GetUVError(iret);
		//        // LOGE(errmsg_);
		return false;
	}
	return true;
}

void GetPacket(const NetPacket& packethead, const unsigned char* packetdata, void* userdata)
{
	fprintf(stdout, "Get control packet type %d\n", packethead.type);
	assert(userdata);
	TcpClientCtx* theclass = (TcpClientCtx*)userdata;
	CTcpHandle* parent = (CTcpHandle*)theclass->parent_server;
	/*const std::string& senddata = parent->protocol_->ParsePacket(packethead, packetdata);
	parent->sendinl(senddata, theclass);*/
	parent->ParsePacket(packethead, packetdata, theclass);
	return;
}
