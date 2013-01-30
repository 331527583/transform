#include "CRoomMgr.h"


CRoomMgr* CRoomMgr::m_pInstance = NULL;

CRoomMgr::CRoomMgr()
{

}

bool CRoomMgr::changeRoomer(const string &sType,const string &sRoomId,const string &sName)
{
	
	SRoom *pstRoom = get(sType,sRoomId);

	if(pstRoom)
	{
		if(pstRoom->sRoomer == sName)
		{
			if(!pstRoom->setPlayers.empty())
			{
				pstRoom->sRoomer = (*pstRoom->setPlayers.begin());
			}
			else
			{
				pstRoom->sRoomer = "";
			}
		}

	}
	
	return true;
}

bool CRoomMgr::exist(const string &sType,const string &sRoomId)
{

	TYPE_ROOM::iterator it = m_mapRooms.find(sType);
	if(it != m_mapRooms.end())
	{
		MAP_ROOM::iterator it2 = it->second->find(sRoomId);
		if(it2 != it->second->end())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	return false;
}

bool CRoomMgr::del(const string &sType,const string &sRoomId)
{

	TYPE_ROOM::iterator it = m_mapRooms.find(sType);
	if(it != m_mapRooms.end())
	{
		MAP_ROOM::iterator it2 = it->second->find(sRoomId);
		if(it2 != it->second->end())
		{
			delete it2->second;
			it2->second = NULL;
			it->second->erase(it2);
			return true;
		}
	}
	return true;

}

SRoom * CRoomMgr::add(const string &sType,const string &sRoomId)
{
	TYPE_ROOM::iterator it = m_mapRooms.find(sType);
	if(it != m_mapRooms.end())
	{
		MAP_ROOM::iterator it2 = it->second->find(sRoomId);

		if(it2 != it->second->end())
		{
			return NULL;
		}
		else
		{
			SRoom *pstRoom = new SRoom;
			pstRoom->sRoomId = sRoomId;
			pstRoom->sType = sType;
			it->second->insert(std::pair<string,SRoom*>(sRoomId,pstRoom));
			return pstRoom;
		}
	}
	else
	{
		MAP_ROOM *pmapRoom = new MAP_ROOM;
		m_mapRooms[sType] = pmapRoom;

		SRoom *pstRoom = new SRoom;
		pstRoom->sRoomId = sRoomId;
		pstRoom->sType = sType;
		pmapRoom->insert(std::pair<string,SRoom*>(sRoomId,pstRoom));
		return pstRoom;
	}
}

SRoom *CRoomMgr::get(const string &sType,const string &sRoomId)
{
	TYPE_ROOM::iterator it = m_mapRooms.find(sType);
	if(it != m_mapRooms.end())
	{
		MAP_ROOM::iterator it2 = it->second->find(sRoomId);

		if(it2 != it->second->end())
		{
			return it2->second;
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		return NULL;
	}

}



bool CRoomMgr::join(const string &sType,const string &sRoomId,const string &sName)
{
	TYPE_ROOM::iterator it = m_mapRooms.find(sType);
	if(it != m_mapRooms.end())
	{
		MAP_ROOM::iterator it2 = it->second->find(sRoomId);

		if(it2 != it->second->end())
		{
			it2->second->setPlayers.insert(sName);
			return true;
		}
		else
		{
			return false;
		}
	}
	return false;
}

bool CRoomMgr::exit(const string &sType,const string &sRoomId,const string &sName)
{
	TYPE_ROOM::iterator it = m_mapRooms.find(sType);
	if(it != m_mapRooms.end())
	{
		MAP_ROOM::iterator it2 = it->second->find(sRoomId);

		if(it2 != it->second->end())
		{
			it2->second->setPlayers.erase(sName);
			return true;
		}
		else
		{
			return true;
		}
	}
	return true;
}

bool CRoomMgr::isInRoom(const string &sType,const string &sRoomId,const string &sName)
{

	TYPE_ROOM::iterator it = m_mapRooms.find(sType);
	if(it != m_mapRooms.end())
	{
		MAP_ROOM::iterator it2 = it->second->find(sRoomId);

		if(it2 != it->second->end())
		{
			set<string>::iterator it3 = it2->second->setPlayers.find(sName);
			if(it3 != it2->second->setPlayers.end())
			{
				return true;
			}
			return false;
		}
		else
		{
			return false;
		}
	}
	return false;

}