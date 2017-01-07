#ifndef TCP_HANDLE_H_23432358247985439
#define TCP_HANDLE_H_23432358247985439

#include <string>
#include "simple_uv_export.h"
#include "uv.h"
#include "BaseMsgDefine.h"

using namespace uv;
using namespace std;

#define SERVER_PACKET_HEAD 0x01
#define SERVER_PACKET_TAIL 0x02

#define BEGIN_UV_BIND virtual int  ParsePacket(const NetPacket& packet, const unsigned char* buf, TcpClientCtx *pClient) \
	{ \

#define UV_BIND(MSG_TYPE, MSG_CLASS) \
			if (MSG_TYPE == packet.type) { \
				MSG_CLASS *msg = (MSG_CLASS *)buf; \
				return this->OnUvMessage(*msg, pClient); \
				} \

#define END_UV_BIND(BASE_CLASS) \
			return BASE_CLASS::ParsePacket(packet, buf, pClient); \
	} \

class CTcpHandle
{
public:
	CTcpHandle();
	~CTcpHandle(void);

	virtual void SUV_EXPORT Close();//send close command. verify IsClosed for real closed
	virtual int SUV_EXPORT ParsePacket(const NetPacket& packet, const unsigned char* buf, TcpClientCtx *pClient);
	bool SUV_EXPORT IsClosed() {//verify if real closed
		return isclosed_;
	};

	//Enable or disable Nagle¡¯s algorithm. must call after Server succeed start.
	bool SUV_EXPORT SetNoDelay(bool enable);

	//Enable or disable KeepAlive. must call after Server succeed start.
	//delay is the initial delay in seconds, ignored when enable is zero
	bool SUV_EXPORT SetKeepAlive(int enable, unsigned int delay);

	const SUV_EXPORT char* GetLastErrMsg() const {
		return errmsg_.c_str();
	};

protected:
	virtual bool init();
	virtual void SUV_EXPORT send_inl(uv_write_t* req = NULL);//real send data fun
	virtual void closeinl();//real close fun
	bool run(int status = UV_RUN_DEFAULT);

	template<class TYPE>
	string PacketData(const TYPE& msg, size_t nMsgType);

	bool isclosed_;
	bool isuseraskforclosed_;
	char packet_head;//protocol head
	char packet_tail;//protocol tail
	uv_async_t async_handle_;
	uv_tcp_t tcp_handle_;
	uv_loop_t loop_;
	std::string errmsg_;
	uv_mutex_t mutex_clients_;//clients map mutex

	enum {
		CONNECT_TIMEOUT,
		CONNECT_FINISH,
		CONNECT_ERROR,
		CONNECT_DIS
	};

private:
	static void AsyncCloseCB(uv_async_t* handle);//async close
	
};

template<class TYPE>
string CTcpHandle::PacketData(const TYPE& msg, size_t nMsgType)
{
	NetPacket tmppack;
	tmppack.type = nMsgType;
	tmppack.header = SERVER_PACKET_HEAD;
	tmppack.tail = SERVER_PACKET_TAIL;
	tmppack.datalen = sizeof(TYPE);
	return PacketData2(tmppack, (const unsigned char*)&msg);
}

void GetPacket(const NetPacket& packethead, const unsigned char* packetdata, void* userdata);

#endif
