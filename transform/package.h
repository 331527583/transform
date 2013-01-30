#ifndef __PACKAGE_H__
#define __PACKAGE_H__

#include "lce.h"
using namespace lce;



enum CMD_TRANSFORM
{
	CMD_LOGIN = 1001,//登录
	CMD_HEART_BEAT=1003,//心跳检测
	CMD_JOIN_ROOM = 1004,//加入房间
	CMD_EXIT_ROOM=1005,//离开房间
	CMD_KICK_ROOM=1006,//踢某人出房间
	CMD_KICK_ROOM_DATA=1007,//通知某人被踢出房间
	CMD_BROADCAST_ROOM = 1008,//玩家向服务器房间广播数据
	CMD_BROADCAST_ROOM_DATA = 1009,//服务器向房间客户端广播数据
	CMD_BROADCAST_SERVER =1010,//向整个全服用户广播数据
	CMD_BROADCAST_SERVER_DATA = 1011,//服务器向所有客户端广播数据
	CMD_BROADCAST_SOMEBODY = 1012,//玩家向服务端某些人广播数据
	CMD_BROADCAST_SOMEBODY_DATA = 1013,//服务器向指定某些人广播数据
	CMD_CHANGE_ROOM_DATA = 1018,//房间信息更改广播
	CMD_QUICK_START = 1021, //快速开始
	CMD_GET_ROOMS_INFO = 1022, //获取房间列表信息
	CMD_GET_ROOM_INFO = 1023, //获取某个房间的信息

};


#pragma pack(1)

struct SHead
{
public:
	void setStx(){ m_cStx = 0x2; }
	void setLen(uint32_t dwLen){ m_dwLen = htonl(dwLen); }
	void setCmd(uint16_t wCmd){m_wCmd = htons(wCmd);}
	void setSeq(uint32_t dwSeq){ m_dwSeq = htonl(dwSeq); }
	void setSig(const string & sSig){ strncpy(m_szSig,sSig.data(),32);}

	uint32_t getLen(){ return ntohl(m_dwLen); }
	uint16_t getCmd(){ return ntohs(m_wCmd); }
	uint32_t getSeq(){ return ntohl(m_dwSeq); }
	string getSig(){ return string(m_szSig,32);}

private:
	uint8_t m_cStx; //包起始格试
	uint32_t m_dwLen;//包长度
	uint16_t m_wCmd;//命令字
	uint32_t m_dwSeq;//请求序列
	char m_szSig[32];//请求包体签名
};

#pragma pack()

typedef CAnyValuePackage<SHead> CPkgTransform;


#endif