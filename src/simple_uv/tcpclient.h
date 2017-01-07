﻿/***************************************
* @file     tcpclient.h
* @brief    基于libuv封装的tcp服务器与客户端,使用log4z作日志工具
* @details
* @author   phata, wqvbjhc@gmail.com
* @date     2014-05-13
* @mod      2014-05-13  phata  修正服务器与客户端的错误.现服务器支持多客户端连接
修改客户端测试代码，支持并发多客户端测试
2014-05-23  phata  原服务器与客户端只接收裸数据，现改为接收NetPacket(定义net_base.h)封装的数据。接收回调为解析后的数据，但发送需要用户自己封闭成NetPacket后发送
修改server_recvcb的定义，添加NetPacket参数
修改client_recvcb的定义，添加NetPacket参数
申请uv_write_t列表空间用于send
2014-05-27  phata  clientdata更名为AcceptClient，并丰富了其功能.
使用异步发送机制，可以在其他线程中调用服务器与客户端的send函数
修改之前测试发现崩溃的情况
BUFFER_SIZE由1M改为10K，一个client需要6倍BUFFER_SIZE.一个client内部会启动2个线程
2014-07-24  phata  从tcpsocket中分离出TCPClient。
单独线程实现libuv的run(事件循环)，任何libuv相关操作都在此线程中完成。因此TCPClient可以多线程中任意调用
一个client需要4倍BUFFER_SIZE(readbuffer_,writebuffer_,writebuf_list_,readpacket_),启动两个线程(readpacket_内部一个，Connect启动一个)
2014-11-01  phata  由于运行起来CPU负荷高，决定改进：
1.去掉prepare,check,idle事件
2.prepare里的判断用户关闭tcp和发送数据由uv_async_send代替
3.prepare里的删除多余空闲handle,write_t不需要。回收空闲handle,write_t时判断是否多出预计，多时不回收，直接释放。
2014-11-16  phata  修改发送数据uv_async_send逻辑，现在发送不延时
2015-01-06  phata  使用uv_walk关闭各handle,整个loop关闭回调在run返回后触发。
加入断线重连功能
****************************************/
#ifndef TCPCLIENT_H
#define TCPCLIENT_H
#include <string>
#include <list>
#include "uv.h"
#include "packet_sync.h"
#include "pod_circularbuffer.h"
#include "BaseMsgDefine.h"
#include "simple_uv_export.h"
#include "TcpHandle.h"
#ifndef BUFFER_SIZE
#define BUFFER_SIZE (1024*10)
#endif

namespace uv
{
	class TCPClient : public CTcpHandle
	{
	public:
		SUV_EXPORT TCPClient();
		virtual SUV_EXPORT ~TCPClient();
		//Start/Stop the log
		static SUV_EXPORT void StartLog(const char* logpath = nullptr);
		static SUV_EXPORT void StopLog();
	public:
		bool SUV_EXPORT Connect(const char* ip, int port);//connect the server, ipv4
		bool SUV_EXPORT Connect6(const char* ip, int port);//connect the server, ipv6
		int  SUV_EXPORT Send(const char* data, std::size_t len);//send data to server
		void SUV_EXPORT Close();//send close command. verify IsClosed for real closed
		virtual int SUV_EXPORT ParsePacket(const NetPacket& packet, const unsigned char* buf, TcpClientCtx *pClient);

		template<class TYPE>
		int SendUvMessage(const TYPE& msg, size_t nMsgType);
		
		//Enable or disable Nagle’s algorithm. must call after Server succeed start.
		// bool SUV_EXPORT SetNoDelay(bool enable);

		//Enable or disable KeepAlive. must call after Server succeed start.
		//delay is the initial delay in seconds, ignored when enable is zero
		// bool SUV_EXPORT SetKeepAlive(int enable, unsigned int delay);

		
	protected:
		bool SUV_EXPORT init();
		void SUV_EXPORT closeinl();//real close fun
		void ReConnectCB(NET_EVENT_TYPE eventtype = NET_EVENT_TYPE_RECONNECT);
		// bool run(int status = UV_RUN_DEFAULT);
		void SUV_EXPORT send_inl(uv_write_t* req = NULL);//real send data fun
		static void ConnectThread(void* arg);//connect thread,run until use close the client

		static void AfterConnect(uv_connect_t* handle, int status);
		static void AfterRecv(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf);
		static void AfterSend(uv_write_t* req, int status);
		static void AllocBufferForRecv(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
		static void AfterClientClose(uv_handle_t* handle);
		// static void AsyncCB(uv_async_t* handle);//async close
		static void CloseWalkCB(uv_handle_t* handle, void* arg);//close all handle in loop
		static void ReconnectTimer(uv_timer_t* handle);
		void CloseCB(int clientid, void* userdata);

	private:
		TcpClientCtx *client_handle_;
		// uv_async_t async_handle_;
		// uv_loop_t loop_;
		// bool isuseraskforclosed_;

		uv_thread_t connect_threadhandle_;
		uv_connect_t connect_req_;

		int connectstatus_;

		//send param
		// uv_mutex_t mutex_clients_;//mutex of writebuf_list_
		std::list<write_param*> writeparam_list_;//Availa write_t
		PodCircularBuffer<char> write_circularbuf_;//the data prepare to send

		
		bool StartReconnect(void);
		void StopReconnect(void);
		uv_timer_t reconnect_timer_;
		bool isreconnecting_;
		int64_t repeat_time_;//repeat reconnect time. y=2x(x=1..)

		std::string connectip_;
		int connectport_;
		bool isIPv6_;
		// std::string errmsg_;

		// char PACKET_HEAD;//protocol head
		// char PACKET_TAIL;//protocol tail
	};
}

#endif // TCPCLIENT_H

template<class TYPE>
int uv::TCPClient::SendUvMessage(const TYPE& msg, size_t nMsgType)
{
	string str = this->PacketData(msg, nMsgType);
	return this->Send(&str[0], str.length());
}
