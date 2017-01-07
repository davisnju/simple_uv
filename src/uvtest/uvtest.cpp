#include "stdafx.h"
#include <iostream>
#include <string>
#include "tcpserver.h"
#include "DumpFile.h"

using namespace std;
using namespace uv;
bool is_eist = false;
int call_time = 0;

TCPServer server(SERVER_PACKET_HEAD, SERVER_PACKET_TAIL);

void CloseCB(int clientid, void* userdata)
{
	fprintf(stdout,"cliend %d close\n",clientid);
	TCPServer *theclass = (TCPServer *)userdata;
	//is_eist = true;
}

void NewConnect(int clientid, void* userdata)
{
	fprintf(stdout,"new connect:%d\n",clientid);
	server.SetRecvCB(clientid,NULL,NULL);
}

int main(int argc, char** argv)
{
	DeclareDumpFile();
	TCPServer::StartLog("log/");
	server.SetNewConnectCB(NewConnect,&server);
	if(!server.Start("0.0.0.0",12345)) {
		fprintf(stdout,"Start Server error:%s\n",server.GetLastErrMsg());
	}
	server.SetKeepAlive(1,60);//enable Keepalive, 60s
	fprintf(stdout,"server return on main.\n");
	while(!is_eist) {
		Sleep(10);
	}
	return 0;
}