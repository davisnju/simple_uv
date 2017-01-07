// testclient.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <iostream>
#include <string>
#include "Test.h"
#include "DumpFile.h"
using namespace std;
using namespace uv;

std::string serverip;
int call_time = 0;
bool is_exist = false;


int main(int argc, char** argv)
{
	if (argc != 3) {
		fprintf(stdout, "usage: %s server_ip_address clientcount\neg.%s 192.168.1.1 50\n", argv[0], argv[0]);
		return 0;
	}
	DeclareDumpFile();
	serverip = argv[1];

	const int clientsize = std::stoi(argv[2]);
	CTest pClients;

	if (!pClients.Connect(serverip.c_str(), 12345)) {
		fprintf(stdout, "connect error:%s\n", pClients.GetLastErrMsg());
	} else {
		fprintf(stdout, "client(%p) connect succeed.\n", &pClients);
	}

	CTestMsg msg;
	msg.m_nSessionID = 1000;
	pClients.SendUvMessage(msg, msg.MSG_ID);
	while (!is_exist) {
		Sleep(10);
	}
	return 0;
}

