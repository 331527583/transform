#ifndef __SESSION_MGR_H__
#define __SESSION_MGR_H__
#include "lce.h"
#include <vector>
#include <iostream>
using namespace std;
using namespace lce;

class CSessionMgr
{
public:
	typedef vector<SSession *> VEC_SESSION;

	bool init(uint32_t dwMaxSize = 50000);

	SSession * add(uint32_t dwSessionId)
	{
		SSession *pstSession = vecSessions[dwSessionId];

		if(pstSession == NULL)
		{
			pstSession = new SSession;
			pstSession->iFd = dwSessionId;
			vecSessions[dwSessionId] = pstSession;
			return pstSession;
		}
		else
		{
			return NULL;
		}
	}

	bool exist(uint32_t dwSessionId)
	{
		SSession *pstSession = vecSessions[dwSessionId];

		if(pstSession == NULL)
		{
			return false;
		}
		return true;
	}

	SSession * get(uint32_t dwSessionId)
	{
		SSession *pstSession = vecSessions[dwSessionId];
		return pstSession;
	}

	bool del(uint32_t dwSessionId)
	{
		SSession *pstSession = vecSessions[dwSessionId];

		if(pstSession != NULL)
		{
			delete pstSession;
			vecSessions[dwSessionId] = NULL;
		}
		return true;
	}

	~CSessionMgr(void);

	static CSessionMgr& getInstance()
	{
		if (NULL == m_pInstance){
			m_pInstance = new CSessionMgr;
		}
		return *m_pInstance;
	}

private:
	CSessionMgr(void);
	CSessionMgr& operator=(const CSessionMgr& rhs);
	CSessionMgr(const CSessionMgr& rhs);
private:
	static CSessionMgr* m_pInstance;
	VEC_SESSION vecSessions;
};

#endif