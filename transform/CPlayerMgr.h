#ifndef __PLAYER_MGR_H__
#define __PLAYER_MGR_H__

#include <string>
#include <map>
#include <stdint.h>
using namespace std;

struct SPlayer
{
	SPlayer()
	{
		dwSessionId = 0;
		cPrivilege = 0;
		dwLoginTime = time(NULL);
	}
	string sName;
	string sRoomId;
	string sRoomType;
	uint32_t dwSessionId;
	uint8_t cPrivilege;
	time_t dwLoginTime;
};

class CPlayerMgr
{
public:
	enum PLAY_PRIV 
	{
		PRIV_NORMAL = 0,
		PRIV_VIP = 1,
		PRIV_ADMIN = 2,
	};
	typedef map<string,SPlayer*> MAP_PLAYER;

	MAP_PLAYER & all(){ return m_mapPlayer; } 

	SPlayer * add(const string &sName);

	bool del(const string &sName);
	SPlayer * get(const string &sName);
	
	bool exist(const string &sName);

	~CPlayerMgr(void);

	static CPlayerMgr& getInstance()
	{
		if (NULL == m_pInstance){
			m_pInstance = new CPlayerMgr;
		}
		return *m_pInstance;
	}

private:
	CPlayerMgr(void);
	CPlayerMgr& operator=(const CPlayerMgr& rhs);
	CPlayerMgr(const CPlayerMgr& rhs);
private:
	static CPlayerMgr* m_pInstance;
	MAP_PLAYER m_mapPlayer;
};

#endif