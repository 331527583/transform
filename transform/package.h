#ifndef __PACKAGE_H__
#define __PACKAGE_H__

#include "lce.h"
using namespace lce;



enum CMD_TRANSFORM
{
	CMD_LOGIN = 1001,//��¼
	CMD_HEART_BEAT=1003,//�������
	CMD_JOIN_ROOM = 1004,//���뷿��
	CMD_EXIT_ROOM=1005,//�뿪����
	CMD_KICK_ROOM=1006,//��ĳ�˳�����
	CMD_KICK_ROOM_DATA=1007,//֪ͨĳ�˱��߳�����
	CMD_BROADCAST_ROOM = 1008,//��������������㲥����
	CMD_BROADCAST_ROOM_DATA = 1009,//�������򷿼�ͻ��˹㲥����
	CMD_BROADCAST_SERVER =1010,//������ȫ���û��㲥����
	CMD_BROADCAST_SERVER_DATA = 1011,//�����������пͻ��˹㲥����
	CMD_BROADCAST_SOMEBODY = 1012,//���������ĳЩ�˹㲥����
	CMD_BROADCAST_SOMEBODY_DATA = 1013,//��������ָ��ĳЩ�˹㲥����
	CMD_CHANGE_ROOM_DATA = 1018,//������Ϣ���Ĺ㲥
	CMD_QUICK_START = 1021, //���ٿ�ʼ
	CMD_GET_ROOMS_INFO = 1022, //��ȡ�����б���Ϣ
	CMD_GET_ROOM_INFO = 1023, //��ȡĳ���������Ϣ

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
	uint8_t m_cStx; //����ʼ����
	uint32_t m_dwLen;//������
	uint16_t m_wCmd;//������
	uint32_t m_dwSeq;//��������
	char m_szSig[32];//�������ǩ��
};

#pragma pack()

typedef CAnyValuePackage<SHead> CPkgTransform;


#endif