#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <stdint.h>
#include "../tinyxml.h"

using namespace std;

struct SRoomInfo
{
	string sRoomId;
	uint16_t wMaxNum;
};

typedef vector<SRoomInfo> VEC_ROOM_INFO;

typedef map<string,VEC_ROOM_INFO> MAP_ROOM_INFO;


void getRoomTypesFromXml(TiXmlElement *root,set<string> &setTypes)
{

	TiXmlElement *childNode = root->FirstChildElement();
	
	while( childNode )
	{
		setTypes.insert(childNode->Value());
		childNode =childNode->NextSiblingElement();
	}

}


void getRoomsFromXml(TiXmlElement *root,MAP_ROOM_INFO &mapRoomInfos)
{
	TiXmlElement *childNode = root->FirstChildElement();

	while( childNode )
	{
		string sType = childNode->Value();
		
		TiXmlElement *infoNode = childNode->FirstChildElement();
		while(infoNode)
		{

			SRoomInfo stInfo;
			stInfo.wMaxNum = 0;

			infoNode->QueryStringAttribute("id",&stInfo.sRoomId);
			infoNode->QueryIntAttribute("max_num",(int*)&stInfo.wMaxNum);
			
			if(!stInfo.sRoomId.empty())
			{
				mapRoomInfos[sType].push_back(stInfo);
			}
					
			infoNode = infoNode->NextSiblingElement();
		}
		
		childNode =childNode->NextSiblingElement();
	}
}

void processNode(TiXmlElement *node)
{
	if(node == NULL)
		return;

	cout<<node->Value()<<endl;

	TiXmlAttribute *attr = node->FirstAttribute();

	while(attr)
	{
		cout<<attr->Name()<<":"<<attr->Value()<<"  ";
		attr = attr->Next();
	}
	cout<<endl;
	TiXmlElement *childNode = node->FirstChildElement();
	while( childNode )
	{
		processNode(childNode);
		childNode =childNode->NextSiblingElement();
	}
}


int main(int argc,char *argv[])
{
	TiXmlDocument doc("../rooms.xml");
	doc.LoadFile(); 

	TiXmlElement *roomNode = doc.RootElement();
	processNode(roomNode);
	MAP_ROOM_INFO mapRoomInfos;
	getRoomsFromXml(roomNode,mapRoomInfos);

	for(MAP_ROOM_INFO::iterator it = mapRoomInfos.begin();it!=mapRoomInfos.end();++it)
	{
		cout<<it->first<<endl;

		for(VEC_ROOM_INFO::iterator it2=it->second.begin();it2 != it->second.end();++it2)
		{
			cout<<it2->sRoomId<<" "<<it2->wMaxNum<<endl;
		}
	}
	return 0;
}