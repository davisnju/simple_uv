#include "stdafx.h"
#include "tcpclient.h"
#include "thread_uv.h"
// #include "log4z.h"
#define MAXLISTSIZE 20

namespace uv
{
/*****************************************TCP Client*************************************************************/
TCPClient::TCPClient()
    : connectstatus_(CONNECT_DIS), write_circularbuf_(BUFFER_SIZE)
    , isIPv6_(false), isreconnecting_(false)
{
    client_handle_ = AllocTcpClientCtx(this);
    connect_req_.data = this;
}


TCPClient::~TCPClient()
{
    Close();
    uv_thread_join(&connect_threadhandle_);
    FreeTcpClientCtx(client_handle_);
    for (auto it = writeparam_list_.begin(); it != writeparam_list_.end(); ++it) {
        FreeWriteParam(*it);
    }
    writeparam_list_.clear();

    // LOGI("client(" << this << ")exit");
}

bool TCPClient::init()
{
	if (CTcpHandle::init())
	{
		client_handle_->tcphandle = this->m_tcpHandle;
		client_handle_->tcphandle.data = client_handle_;
		client_handle_->parent_server = this;

		client_handle_->packet_->SetPacketCB(GetPacket, client_handle_);
		client_handle_->packet_->Start(m_cPacketHead, m_cPacketTail);

		int iret = uv_timer_init(&m_loop, &reconnect_timer_);
		if (iret) {
			m_strErrMsg = GetUVError(iret);
			// // LOGI(errmsg_);
			return false;
		}
		reconnect_timer_.data = this;

		return true;
	}

	return false;
}

void TCPClient::closeinl()
{
    if (m_bIsClosed) {
        return;
    }
    StopReconnect();
    uv_walk(&m_loop, CloseWalkCB, this);
    // LOGI("client(" << this << ")close");
}

void TCPClient::ReConnectCB(NET_EVENT_TYPE eventtype)
{
	TCPClient* client = this;
	if (NET_EVENT_TYPE_RECONNECT == eventtype) {
	}
	else {
		fprintf(stdout, "server disconnect.\n");
	}
}


bool TCPClient::Connect(const char* ip, int port)
{
    closeinl();
    if (!init()) {
        return false;
    }
    connectip_ = ip;
    connectport_ = port;
    isIPv6_ = false;
    struct sockaddr_in bind_addr;
    int iret = uv_ip4_addr(connectip_.c_str(), connectport_, &bind_addr);
    if (iret) {
        m_strErrMsg = GetUVError(iret);
        // // LOGI(errmsg_);
        return false;
    }
    iret = uv_tcp_connect(&connect_req_, &client_handle_->tcphandle, (const sockaddr*)&bind_addr, AfterConnect);
    if (iret) {
        m_strErrMsg = GetUVError(iret);
        // // LOGI(errmsg_);
        return false;
    }

    // LOGI("client(" << this << ")start connect to server(" << ip << ":" << port << ")");
    iret = uv_thread_create(&connect_threadhandle_, ConnectThread, this);//thread to wait for succeed connect.
    if (iret) {
        m_strErrMsg = GetUVError(iret);
        // // LOGI(errmsg_);
        return false;
    }
    int wait_count = 0;
    while (connectstatus_ == CONNECT_DIS) {
        ThreadSleep(100);
        if (++wait_count > 100) {
            connectstatus_ = CONNECT_TIMEOUT;
            break;
        }
    }
    if (CONNECT_FINISH != connectstatus_) {
        m_strErrMsg = "connect time out";
        return false;
    } else {
        return true;
    }
}

bool TCPClient::Connect6(const char* ip, int port)
{
    closeinl();
    if (!init()) {
        return false;
    }
    connectip_ = ip;
    connectport_ = port;
    isIPv6_ = true;
    struct sockaddr_in6 bind_addr;
    int iret = uv_ip6_addr(connectip_.c_str(), connectport_, &bind_addr);
    if (iret) {
        m_strErrMsg = GetUVError(iret);
        // // LOGI(errmsg_);
        return false;
    }
    iret = uv_tcp_connect(&connect_req_, &client_handle_->tcphandle, (const sockaddr*)&bind_addr, AfterConnect);
    if (iret) {
        m_strErrMsg = GetUVError(iret);
        // // LOGI(errmsg_);
        return false;
    }

    // LOGI("client(" << this << ")start connect to server(" << ip << ":" << port << ")");
    iret = uv_thread_create(&connect_threadhandle_, ConnectThread, this);//thread to wait for succeed connect.
    if (iret) {
        m_strErrMsg = GetUVError(iret);
        // // LOGI(errmsg_);
        return false;
    }
    int wait_count = 0;
    while (connectstatus_ == CONNECT_DIS) {
        ThreadSleep(100);
        if (++wait_count > 100) {
            connectstatus_ = CONNECT_TIMEOUT;
            break;
        }
    }
    if (CONNECT_FINISH != connectstatus_) {
        m_strErrMsg = "connect time out";
        return false;
    } else {
        return true;
    }
}

void TCPClient::ConnectThread(void* arg)
{
    TCPClient* pclient = (TCPClient*)arg;
    pclient->run();
}

void TCPClient::AfterConnect(uv_connect_t* handle, int status)
{
    TcpClientCtx* theclass = (TcpClientCtx*)handle->handle->data;
    TCPClient* parent = (TCPClient*)theclass->parent_server;
    if (status) {
        parent->connectstatus_ = CONNECT_ERROR;
        parent->m_strErrMsg = GetUVError(status);
        // // LOGI("client(" << parent << ") connect error:" << parent->errmsg_);
        fprintf(stdout, "connect error:%s\n", parent->m_strErrMsg.c_str());
        if (parent->isreconnecting_) {//reconnect failure, restart timer to trigger reconnect.
            uv_timer_stop(&parent->reconnect_timer_);
            parent->repeat_time_ *= 2;
            uv_timer_start(&parent->reconnect_timer_, TCPClient::ReconnectTimer, parent->repeat_time_, parent->repeat_time_);
        }
        return;
    }

    int iret = uv_read_start(handle->handle, AllocBufferForRecv, AfterRecv);
    if (iret) {
        parent->m_strErrMsg = GetUVError(status);
        // // LOGI("client(" << parent << ") uv_read_start error:" << parent->errmsg_);
        fprintf(stdout, "uv_read_start error:%s\n", parent->m_strErrMsg.c_str());
        parent->connectstatus_ = CONNECT_ERROR;
    } else {
        parent->connectstatus_ = CONNECT_FINISH;
        // LOGI("client(" << parent << ")run");
    }
    if (parent->isreconnecting_) {
        fprintf(stdout, "reconnect succeed\n");
        parent->StopReconnect();//reconnect succeed.
        
		parent->ReConnectCB(NET_EVENT_TYPE_RECONNECT);
    }
}

int TCPClient::Send(const char* data, std::size_t len)
{
    if (!data || len <= 0) {
        m_strErrMsg = "send data is null or len less than zero.";
        // // LOGI(errmsg_);
        return 0;
    }
    uv_async_send(&m_asyncHandle);
    size_t iret = 0;
    while (!m_bIsUserAskForClosed) {
        uv_mutex_lock(&m_mutexClients);
        iret += write_circularbuf_.write(data + iret, len - iret);
        uv_mutex_unlock(&m_mutexClients);
        if (iret < len) {
            ThreadSleep(100);
            continue;
        } else {
            break;
        }
    }
    uv_async_send(&m_asyncHandle);
    return iret;
}


void TCPClient::AllocBufferForRecv(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
    TcpClientCtx* theclass = (TcpClientCtx*)handle->data;
    assert(theclass);
    *buf = theclass->read_buf_;
}

void TCPClient::AfterRecv(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf)
{
    TcpClientCtx* theclass = (TcpClientCtx*)handle->data;
    assert(theclass);
    TCPClient* parent = (TCPClient*)theclass->parent_server;
    if (nread < 0) {
		parent->ReConnectCB(NET_EVENT_TYPE_DISCONNECT);

        if (!parent->StartReconnect()) {
            fprintf(stdout, "Start Reconnect Failure.\n");
            return;
        }
        if (nread == UV_EOF) {
            fprintf(stdout, "Server close(EOF), Client %p\n", handle);
            // LOGI(("Server close(EOF)");
        } else if (nread == UV_ECONNRESET) {
            fprintf(stdout, "Server close(conn reset),Client %p\n", handle);
            // LOGI(("Server close(conn reset)");
        } else {
            fprintf(stdout, "Server close,Client %p:%s\n", handle, GetUVError(nread));
            // LOGI(("Server close" << GetUVError(nread));
        }
        uv_close((uv_handle_t*)handle, AfterClientClose);//close before reconnect
        return;
    }
    parent->send_inl(NULL);
    if (nread > 0) {
        theclass->packet_->recvdata((const unsigned char*)buf->base, nread);
    }
}

void TCPClient::AfterSend(uv_write_t* req, int status)
{
    TCPClient* theclass = (TCPClient*)req->data;
    if (status < 0) {
        if (theclass->writeparam_list_.size() > MAXLISTSIZE) {
            FreeWriteParam((write_param*)req);
        } else {
            theclass->writeparam_list_.push_back((write_param*)req);
        }
        // // LOGI("send error:" << GetUVError(status));
        fprintf(stderr, "send error %s\n", GetUVError(status));
        return;
    }
    theclass->send_inl(req);
}

/* Fully close a loop */
void TCPClient::CloseWalkCB(uv_handle_t* handle, void* arg)
{
    TCPClient* theclass = (TCPClient*)arg;
    if (!uv_is_closing(handle)) {
        uv_close(handle, AfterClientClose);
    }
}

void TCPClient::AfterClientClose(uv_handle_t* handle)
{
    TCPClient* theclass = (TCPClient*)handle->data;
    fprintf(stdout, "Close CB handle %p\n", handle);
    if (handle == (uv_handle_t*)&theclass->client_handle_->tcphandle && theclass->isreconnecting_) {
        //closed, start reconnect timer
        int iret = 0;
        iret = uv_timer_start(&theclass->reconnect_timer_, TCPClient::ReconnectTimer, theclass->repeat_time_, theclass->repeat_time_);
        if (iret) {
            uv_close((uv_handle_t*)&theclass->reconnect_timer_, TCPClient::AfterClientClose);
            // // LOGI(GetUVError(iret));
            return;
        }
    }
}

void TCPClient::StartLog(const char* logpath /*= nullptr*/)
{
    /*zsummer::log4z::ILog4zManager::GetInstance()->SetLoggerMonthdir(LOG4Z_MAIN_LOGGER_ID, true);
    zsummer::log4z::ILog4zManager::GetInstance()->SetLoggerDisplay(LOG4Z_MAIN_LOGGER_ID, false);
    zsummer::log4z::ILog4zManager::GetInstance()->SetLoggerLevel(LOG4Z_MAIN_LOGGER_ID, LOG_LEVEL_DEBUG);
    zsummer::log4z::ILog4zManager::GetInstance()->SetLoggerLimitSize(LOG4Z_MAIN_LOGGER_ID, 100);
    if (logpath) {
        zsummer::log4z::ILog4zManager::GetInstance()->SetLoggerPath(LOG4Z_MAIN_LOGGER_ID, logpath);
    }
    zsummer::log4z::ILog4zManager::GetInstance()->Start();*/
}

void TCPClient::StopLog()
{
    // zsummer::log4z::ILog4zManager::GetInstance()->Stop();
}


/*
void TCPClient::AsyncCB(uv_async_t* handle)
{
    TCPClient* theclass = (TCPClient*)handle->data;
    if (theclass->isuseraskforclosed_) {
        theclass->closeinl();
        return;
    }
    //check data to send
    theclass->send_inl(NULL);
}*/

void TCPClient::send_inl(uv_write_t* req /*= NULL*/)
{
    write_param* writep = (write_param*)req;
    if (writep) {
        if (writeparam_list_.size() > MAXLISTSIZE) {
            FreeWriteParam(writep);
        } else {
            writeparam_list_.push_back(writep);
        }
    }
    while (true) {
        uv_mutex_lock(&m_mutexClients);
        if (write_circularbuf_.empty()) {
            uv_mutex_unlock(&m_mutexClients);
            break;
        }
        if (writeparam_list_.empty()) {
            writep = AllocWriteParam();
            writep->write_req_.data = this;
        } else {
            writep = writeparam_list_.front();
            writeparam_list_.pop_front();
        }
        writep->buf_.len = write_circularbuf_.read(writep->buf_.base, writep->buf_truelen_); 
        uv_mutex_unlock(&m_mutexClients);
        int iret = uv_write((uv_write_t*)&writep->write_req_, (uv_stream_t*)&client_handle_->tcphandle, &writep->buf_, 1, AfterSend);
        if (iret) {
            writeparam_list_.push_back(writep);//failure not call AfterSend. so recycle req
            // // LOGI("client(" << this << ") send error:" << GetUVError(iret));
            fprintf(stdout, "send error. %s-%s\n", uv_err_name(iret), uv_strerror(iret));
        }
    }
}

int TCPClient::ParsePacket(const NetPacket& packet, const unsigned char* buf, TcpClientCtx *pClient)
{
	char senddata[256] = { 0 };
	TCPClient* client = this;
	sprintf_s(senddata, "****recv server data(%p,%d)", client, packet.datalen);
	fprintf(stdout, "%s\n", senddata);
	NetPacket tmppack = packet;
	tmppack.datalen = (std::min)(strlen(senddata), sizeof(senddata) - 1);
	std::string retstr = PacketData(senddata, 4444);
	if (client->Send(&retstr[0], retstr.length()) <= 0) {
		fprintf(stdout, "(%p)send error.%s\n", client, client->GetLastErrMsg());
	}
	// fprintf(stdout, "call time %d\n", ++call_time);
	return 0;
}

void TCPClient::Close()
{
    if (m_bIsClosed) {
        return;
    }
    m_bIsUserAskForClosed = true;
    uv_async_send(&m_asyncHandle);
}

bool TCPClient::StartReconnect(void)
{
    isreconnecting_ = true;
    client_handle_->tcphandle.data = this;
    repeat_time_ = 1000;//1 sec
    return true;
}

void TCPClient::StopReconnect(void)
{
    isreconnecting_ = false;
    client_handle_->tcphandle.data = client_handle_;
    repeat_time_ = 1000;//1 sec
    uv_timer_stop(&reconnect_timer_);
}

void TCPClient::ReconnectTimer(uv_timer_t* handle)
{
    TCPClient* theclass = (TCPClient*)handle->data;
    if (!theclass->isreconnecting_) {
        return;
    }
    // LOGI("start reconnect...\n");
    do {
        int iret = uv_tcp_init(&theclass->m_loop, &theclass->client_handle_->tcphandle);
        if (iret) {
            // // LOGI(GetUVError(iret));
            break;
        }
        theclass->client_handle_->tcphandle.data = theclass->client_handle_;
        theclass->client_handle_->parent_server = theclass;
        struct sockaddr* pAddr;
        if (theclass->isIPv6_) {
            struct sockaddr_in6 bind_addr;
            int iret = uv_ip6_addr(theclass->connectip_.c_str(), theclass->connectport_, &bind_addr);
            if (iret) {
                // // LOGI(GetUVError(iret));
                uv_close((uv_handle_t*)&theclass->client_handle_->tcphandle, NULL);
                break;
            }
            pAddr = (struct sockaddr*)&bind_addr;
        } else {
            struct sockaddr_in bind_addr;
            int iret = uv_ip4_addr(theclass->connectip_.c_str(), theclass->connectport_, &bind_addr);
            if (iret) {
                // // LOGI(GetUVError(iret));
                uv_close((uv_handle_t*)&theclass->client_handle_->tcphandle, NULL);
                break;
            }
            pAddr = (struct sockaddr*)&bind_addr;
        }
        iret = uv_tcp_connect(&theclass->connect_req_, &theclass->client_handle_->tcphandle, (const sockaddr*)pAddr, AfterConnect);
        if (iret) {
            // // LOGI(GetUVError(iret));
            uv_close((uv_handle_t*)&theclass->client_handle_->tcphandle, NULL);
            break;
        }
        return;
    } while (0);
    //reconnect failure, restart timer to trigger reconnect.
    uv_timer_stop(handle);
    theclass->repeat_time_ *= 2;
    uv_timer_start(handle, TCPClient::ReconnectTimer, theclass->repeat_time_, theclass->repeat_time_);
}

void TCPClient::CloseCB(int clientid, void* userdata)
{
	this->Close();
}

}