/*****************************************
* @file     thread_uv.h
* @brief    对libuv下的线程与锁进行封装
* @details
* @author   phata,wqvbjhc@gmail.com
* @date     2014-10-27
* @mod      2015-03-24  phata  uv_err_name,uv_strerror返回NULL时直接转string会崩溃，先判断
******************************************/
#ifndef COMMON_THREAD_UV_H
#define COMMON_THREAD_UV_H
#include <string>
#include <map>
#include "uv.h"
using namespace std;
#include "simple_uv_export.h"
#include "BaseMsgDefine.h"
#include "simple_locks.h"
#include "uv_msg_framing.h"

//对Android平台，也认为是linux
#ifdef ANDROID
#define __linux__ ANDROID
#endif
//包含头文件
#if defined (WIN32) || defined(_WIN32)
#include <windows.h>
#endif
#ifdef __linux__
#include <pthread.h>
#include <unistd.h>
#endif

//函数
#if defined (WIN32) || defined(_WIN32)
#define uv_thread_close(t) (CloseHandle(t)!=FALSE)
#define uv_thread_sleep(ms) Sleep(ms);//睡眠ms毫秒
#define uv_thread_id GetCurrentThreadId//得到当前线程句柄

#elif defined(__linux__)
#define uv_thread_close(t) ()
#define uv_thread_sleep(ms) usleep((ms) * 1000)
#define uv_thread_id pthread_self//得到当前线程句柄

#else
#error "no supported os"
#endif

/*****************************
* @brief   获取libuv错误码对应的错误信息
* @param   errcode     --libuv函数错误码(不等于0的返回值)
* @return  std::string --返回的详细错误说明
******************************/
inline SUV_EXPORT std::string GetUVError(int errcode)
{
    if (0 == errcode) {
        return "";
    }
    std::string err;
    auto tmpChar = uv_err_name(errcode);
    if (tmpChar) {
        err = tmpChar;
        err += ":";
    }else{
		err = "unknown system errcode "+std::to_string((long long)errcode);
		err += ":";
	}
    tmpChar = uv_strerror(errcode);
    if (tmpChar) {
        err += tmpChar;
    }
    return std::move(err);
}


#define BEGIN_UV_THREAD_BIND virtual void OnDispatchMsg(unsigned int nMsgType, void *data, unsigned int nSrcAddr) \
	{ \

#define UV_THREAD_BIND(MsgType, MSG_CLASS) \
		if (MsgType == nMsgType) \
		{ \
			MSG_CLASS *pMsg = (MSG_CLASS *)data; \
			this->OnUvThreadMessage(*pMsg, nSrcAddr); \
			delete pMsg; pMsg = nullptr; return ; \
		} \

#define END_UV_THREAD_BIND(BASE_CLASS) return BASE_CLASS::OnDispatchMsg(nMsgType, data, nSrcAddr); \
	} \

#define END_BASE_UV_THREAD_BIND return ; } \

class SUV_EXPORT CUVThread
{
public:

	CUVThread(unsigned int nThreadType)
		: m_nThreadType(nThreadType)
        , m_bIsRunning(false)
		, m_pMsgTail(nullptr)
    {
		m_mapThread = new map<unsigned int, CUVThread *>;
    }
    ~CUVThread(void)
    {
        if (m_bIsRunning) {
            uv_thread_join(&m_thread);
        }
        m_bIsRunning = false;
    }
	
    void Start();
    void Stop()
    {
        if (!m_bIsRunning) {
            return;
        }
        uv_thread_join(&m_thread);
        m_bIsRunning = false;
    }
    int GetThreadID(void) const
    {
        return uv_thread_id();
    }
    bool IsRunning(void) const
    {
        return m_bIsRunning;
    }

	template<class TYPE>
	void PushBackMsg(unsigned int nMsgType, const TYPE &msg, unsigned int nSrcAddr = 0);

protected:
	virtual  void Run();
	virtual  int  OnInit();
	virtual  void  OnExit();

	template<class TYPE>
	int SendUvMessage(const TYPE& msg, size_t nMsgType, unsigned int nDstAddr);

	BEGIN_UV_THREAD_BIND
		UV_THREAD_BIND(CRegistMsg::MSG_ID, CRegistMsg)
		UV_THREAD_BIND(UN_REGIST_THREAD_MSG, unsigned int)
	END_BASE_UV_THREAD_BIND
		
	void  OnUvThreadMessage(CRegistMsg msg, unsigned int nSrcAddr);
	void  OnUvThreadMessage(unsigned int msg, unsigned int nSrcAddr);
	unsigned int m_nThreadType;

private:
	void PushBackMsg(NodeMsg *msg);  // 这个函数调用的时候注意传递的值
	CUVThread(){}
	static void ThreadFun(void* arg);
    uv_thread_t m_thread;
	CUVMutex m_lock;
    bool m_bIsRunning;
	NodeMsg *m_pMsgTail;
	map<unsigned int, CUVThread *> *m_mapThread;
};

template<class TYPE>
int CUVThread::SendUvMessage( const TYPE& msg, size_t nMsgType, unsigned int nDstAddr )
{
	map<unsigned int, CUVThread *>::iterator it = m_mapThread->find(nDstAddr);

	if (it == m_mapThread->end())
	{
		return -1;
	}

	it->second->PushBackMsg(nMsgType, msg, m_nThreadType);

	return 0;
}

template<class TYPE>
void CUVThread::PushBackMsg(unsigned int nMsgType, const TYPE &msg, unsigned int nSrcAddr)
{
	NodeMsg *pMsg = new NodeMsg;
	pMsg->m_nMsgType = nMsgType;
	pMsg->m_nSrcAddr = nSrcAddr;
	TYPE *pData = new TYPE(msg);
	pMsg->m_pData = pData;

	this->PushBackMsg(pMsg);
}

#endif //COMMON_THREAD_UV_H
