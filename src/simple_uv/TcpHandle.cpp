#include "stdafx.h"
#include "TcpHandle.h"
#include "thread_uv.h"


CTcpHandle::CTcpHandle(void)
	: isclosed_(true)
	, isuseraskforclosed_(false)
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

void SUV_EXPORT CTcpHandle::Close()
{
	if (isclosed_) {
		return;
	}
}

bool CTcpHandle::init()
{
	if (!isclosed_) {
		return true;
	}
	int iret = uv_async_init(&loop_, &async_handle_close_, AsyncCloseCB);
	if (iret) {
		errmsg_ = GetUVError(iret);
		//        // LOGE(errmsg_);
		return false;
	}
	async_handle_close_.data = this;

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

void SUV_EXPORT CTcpHandle::send_inl( uv_write_t* req /*= NULL*/ )
{

}

void CTcpHandle::closeinl()
{
	
}
