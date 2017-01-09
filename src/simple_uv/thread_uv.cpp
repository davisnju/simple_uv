#include "thread_uv.h"
#include "UVThreadMng.h"

void CUVThread::PushBackMsg(NodeMsg *msg)
{
	m_lock.WriteLock();
	if (msgTail)
	{
		msg->next = msg;
		msgTail = msg;
	}
	else
	{
		msg->next = msgTail->next;
		msgTail->next = msg;
		msgTail = msg;
	}
	m_lock.WriteUnLock();
}

void CUVThread::ThreadFun(void* arg)
{
	CUVThread *pThread = (CUVThread *)arg;

	if (!pThread->OnInit())
	{
		pThread->Run();
	}	

	uv_thread_join(&pThread->thread_);
}

void CUVThread::Run()
{
	NodeMsg* req;
	NodeMsg* first;
	NodeMsg* next;

	while (true)
	{
		if (msgTail == nullptr)
		{
			Sleep(100);
			continue;
		}

		m_lock.WriteLock();
		first = msgTail->next;
		next = first;
		msgTail = nullptr;
		m_lock.WriteUnLock();

		while (next != nullptr) {
			req = next;
			next = req->next != first ? req->next : nullptr;

			this->OnDispatchMsg(req->m_nMsgType, req->m_pData, req->m_nSrcAddr);

			delete req;
			req = nullptr;
		}
	}
}


void CUVThread::OnUvThreadMessage(CRegistMsg msg, unsigned int nSrcAddr)
{
	m_mapThread[msg.m_nType] = (CUVThread *)(msg.m_pData);
}

void CUVThread::OnUvThreadMessage( unsigned int msg, unsigned int nSrcAddr )
{
	map<unsigned int, CUVThread *>::iterator it = m_mapThread.find(msg);

	if (it != m_mapThread.end())
	{
		m_mapThread.erase(it);
	}
}

SUV_EXPORT int CUVThread::OnInit()
{
	CUVThreadMng::GetInstance()->RegistThread(m_nThreadType, this);

	return 0;
}

