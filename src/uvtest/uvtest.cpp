#include <iostream>
#include <string>
#include "TestGateWay.h"
#include "TestServerCenter.h"

using namespace std;
using namespace uv;
bool is_eist = false;
int call_time = 0;



int main(int argc, char** argv)
{
	CTestGateWay server;

	if(!server.Start("0.0.0.0",12345)) {
		fprintf(stdout,"Start Server error:%s\n",server.GetLastErrMsg());
	}
	server.SetKeepAlive(1,60);//enable Keepalive, 60s
	fprintf(stdout,"server return on main.\n");

	CTestServerCenter serverCenter;
	serverCenter.Start();
	while(!is_eist) {
		uv_thread_sleep(10);
	}
	return 0;
}
