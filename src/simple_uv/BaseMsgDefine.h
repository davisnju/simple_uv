#pragma once
#include "uv.h"
#include "packet_sync.h"

namespace uv
{
	// class AcceptClient;
	typedef struct _tcpclient_ctx {
		uv_tcp_t tcphandle;//data filed store this
		PacketSync* packet_;//userdata filed storethis
		uv_buf_t read_buf_;
		int clientid;
		void* parent_server;//tcpserver
		void* parent_acceptclient;//accept client
	} TcpClientCtx;
	TcpClientCtx* AllocTcpClientCtx(void* parentserver);
	void FreeTcpClientCtx(TcpClientCtx* ctx);

	typedef struct _write_param { //the param of uv_write
		uv_write_t write_req_;
		uv_buf_t buf_;
		int buf_truelen_;
	} write_param;
	write_param* AllocWriteParam(void);
	void FreeWriteParam(write_param* param);
}