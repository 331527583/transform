#ifndef __ROOM_MGR_H__
#define __ROOM_MGR_H__

#include <string>
#include <map>
#include <set>
#include <stdint.h>
#include <iostream>
using namespace std;

struct SRoom
{
	SRoom()
	{
		wMaxNum = 0;
		cType = 0;
	}
	string sRoomId;
	string sType;
	string sRoomer;
	uint16_t wMaxNum;
	uint8_t cType;
	set<string> setPlayers;
};

class CRoomMgr
{
public:

	typedef map<string,SRoom*> MAP_ROOM;
	typedef map<string,MAP_ROOM *> TYPE_ROOM;


	SRoom * add(const string &sType,const string &sRoomId);

	bool changeRoomer(const string &sType,const string &sRoomId,const string &sName);

	bool exist(const string &sType,const string &sRoomId);
	bool del(const string &sType,const string &sRoomId);
	SRoom *get(const string &sType,const string &sRoomId);
	bool isInRoom(const string &sType,const string &sRoomId,const string &sName);

	bool join(const string &sType,const string &sRoomId,const string &sName);
	bool exit(const string &sType,const string &sRoomId,const string &sName);

	MAP_ROOM * rooms(const string &sType)
	{
		TYPE_ROOM::iterator it = m_mapRooms.find(sType);
		if(it!= m_mapRooms.end())
		{
			return it->second;
		}
		else
		{
			return NULL;
		}
	}

	~CRoomMgr(void);
	static CRoomMgr& getInstance()
	{
		if (NULL == m_pInstance){
			m_pInstance = new CRoomMgr;
		}
		return *m_pInstance;
	}
private:
	CRoomMgr(void);
	CRoomMgr& operator=(const CRoomMgr& rhs);
	CRoomMgr(const CRoomMgr& rhs);
private:
	static CRoomMgr* m_pInstance;
	TYPE_ROOM m_mapRooms;
};

#endif