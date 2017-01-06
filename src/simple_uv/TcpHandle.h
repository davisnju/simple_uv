#ifndef TCP_HANDLE_H_23432358247985439
#define TCP_HANDLE_H_23432358247985439

#include <string>
#include "simple_uv_export.h"
#include "uv.h"

class CTcpHandle
{
public:
	CTcpHandle(void);
	~CTcpHandle(void);

	virtual void SUV_EXPORT Close();//send close command. verify IsClosed for real closed
	bool SUV_EXPORT IsClosed() {//verify if real closed
		return isclosed_;
	};

	//Enable or disable Nagle��s algorithm. must call after Server succeed start.
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

	bool isclosed_;
	bool isuseraskforclosed_;
	uv_async_t async_handle_close_;
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

#endif
