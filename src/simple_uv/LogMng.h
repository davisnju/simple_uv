#ifndef LOG_MNG_H_234234
#define LOG_MNG_H_234234
#include "simple_uv_export.h"
#include "simple_locks.h"
#include "log4z.h"

class CLogMng
{
public:
	static SUV_EXPORT CLogMng* GetInstance();
	CLogMng(void);
	~CLogMng(void);

	static CLogMng* m_pMng;
	static CUVMutex      m_Mutex;
};


#endif