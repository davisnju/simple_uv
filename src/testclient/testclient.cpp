// testclient.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <iostream>
#include <string>
#include "tcpclient.h"
#include "DumpFile.h"
using namespace std;
using namespace uv;

std::string serverip;
int call_time = 0;
bool is_exist = false;

void CloseCB(int clientid, void* userdata)
{
	fprintf(stdout, "cliend close\n");
	TCPClient* client = (TCPClient*)userdata;
	client->Close();
}

int main(int argc, char** argv)
{
	if (argc != 3) {
		fprintf(stdout, "usage: %s server_ip_address clientcount\neg.%s 192.168.1.1 50\n", argv[0], argv[0]);
		return 0;
	}
	DeclareDumpFile();
	serverip = argv[1];

	const int clientsize = std::stoi(argv[2]);
	TCPClient pClients(SERVER_PACKET_HEAD, SERVER_PACKET_TAIL);
	TCPClient::StartLog("log/");

	int i = 0;
	char senddata[256];
	pClients.SetClosedCB(CloseCB, &pClients);
	if (!pClients.Connect(serverip.c_str(), 12345)) {
		fprintf(stdout, "connect error:%s\n", pClients.GetLastErrMsg());
	} else {
		fprintf(stdout, "client(%p) connect succeed.\n", &pClients);
	}
// 	memset(senddata, 0, sizeof(senddata));
// 	sprintf(senddata, "client(%p) call %d", &pClients, ++call_time);
// 	NetPacket packet;
// 	packet.header = SERVER_PACKET_HEAD;
// 	packet.tail = SERVER_PACKET_TAIL;
// 	packet.datalen = (std::min)(strlen(senddata), sizeof(senddata) - 1);
// 	std::string str = PacketData(packet, (const unsigned char*)senddata);
// 	if (pClients.Send(&str[0], str.length()) <= 0) {
// 		fprintf(stdout, "(%p)send error.%s\n", &pClients, pClients.GetLastErrMsg());
// 	} else {
// 		fprintf(stdout, "send succeed:%s\n", senddata);
// 	}
	pClients.SendUvMessage("aaaaaaaaaaaaa", 5555);
	while (!is_exist) {
		Sleep(10);
	}
	return 0;
}

