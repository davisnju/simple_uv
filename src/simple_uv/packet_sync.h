/***************************************
* @file     packet_sync.h
* @brief    TCP 数据包封装.依赖libuv,openssl.功能：接收数据，解析得到一帧后回调给用户。同步处理，接收到马上解析
* @details  根据net_base.h中NetPacket的定义，对数据包进行封装。
			md5校验码使用openssl函数
			同一线程中实时解码
			长度为0的md5为：d41d8cd98f00b204e9800998ecf8427e，改为全0. 编解码时修改。
//调用方法
Packet packet;
packet.SetPacketCB(GetPacket,&serpac);
packet.Start(0x01,0x02);
//socket有数据到达时，调用packet.recvdata((const unsigned char*)buf,bufsize); 只要足够一帧它会触发GetFullPacket

* @author   phata, wqvbjhc@gmail.com
* @date     2014-05-21
* @mod      2014-08-04 phata 修复解析一帧数据有误的bug
            2014-11-12 phata GetUVError冲突，改为使用thread_uv.h中的
****************************************/
#ifndef PACKET_SYNC_H
#define PACKET_SYNC_H
#include <algorithm>
#include "openssl/md5.h"
#include "net_base.h"
#include "simple_locks.h"//for GetUVError
#if defined (WIN32) || defined(_WIN32)
#include <windows.h>
#define ThreadSleep(ms) Sleep(ms);//睡眠ms毫秒
#elif defined __linux__
#include <unistd.h>
#define ThreadSleep(ms) usleep((ms) * 1000)//睡眠ms毫秒
#endif

#ifdef _MSC_VER
#ifdef NDEBUG
#pragma comment(lib,"libeay32.lib")
#else
#pragma comment(lib,"libeay32MDd.lib")
#endif
#endif
typedef void (*GetFullPacket)(const NetPacket& packethead, const unsigned char* packetdata, void* userdata);

#ifndef BUFFER_SIZE
#define BUFFER_SIZE (1024*10)
#endif


class SUV_EXPORT PacketSync
{
public:
    PacketSync(): packet_cb_(NULL), packetcb_userdata_(NULL) {
        thread_readdata = uv_buf_init((char*)malloc(BUFFER_SIZE), BUFFER_SIZE); //负责从circulebuffer_读取数据
        thread_packetdata = uv_buf_init((char*)malloc(BUFFER_SIZE), BUFFER_SIZE); //负责从circulebuffer_读取packet 中data部分
        truepacketlen = 0;//readdata有效数据长度
        headpos = -1;//找到头位置
        headpt = NULL;//找到头位置
        parsetype = PARSE_NOTHING;
        getdatalen = 0;
    }
    virtual ~PacketSync() {
        free(thread_readdata.base);
        free(thread_packetdata.base);
    }

    bool Start(char packhead, char packtail) {
        HEAD = packhead;
        TAIL = packtail;
        return true;
    }

public:
    void recvdata(const unsigned char* data, size_t len);
    void SetPacketCB(GetFullPacket pfun, void* userdata) {
        packet_cb_ = pfun;
        packetcb_userdata_ = userdata;
    }
private:

    GetFullPacket packet_cb_;//回调函数
    void*         packetcb_userdata_;//回调函数所带的自定义数据

    enum {
        PARSE_HEAD,
        PARSE_NOTHING,
    };
    size_t parsetype;
    size_t getdatalen;
    uv_buf_t  thread_readdata;//负责从circulebuffer_读取数据
    uv_buf_t  thread_packetdata;//负责从circulebuffer_读取packet 中data部分
    size_t truepacketlen;//readdata有效数据长度
    int headpos;//找到头位置
    char* headpt;//找到头位置
    unsigned char HEAD;//包头
    unsigned char TAIL;//包尾
    NetPacket theNexPacket;
    unsigned char md5str[MD5_DIGEST_LENGTH];
private:// no copy
    PacketSync(const PacketSync&);
    PacketSync& operator = (const PacketSync&);
};

/***********************************************辅助函数***************************************************/
/*****************************
* @brief   把数据组合成NetPacket格式的二进制流，可直接发送。
* @param   packet --NetPacket包，里面的version,header,tail,type,datalen,reserve必须提前赋值，该函数会计算check的值。然后组合成二进制流返回
	       data   --要发送的实际数据
* @return  std::string --返回的二进制流。地址：&string[0],长度：string.length()
******************************/
SUV_EXPORT std::string PacketData2(NetPacket& packet, const unsigned char* data);

//客户端或服务器关闭的回调函数
//服务器：当clientid为-1时，表现服务器的关闭事件
//客户端：clientid无效，永远为-1
typedef void (*TcpCloseCB)(int clientid, void* userdata);

//TCPServer接收到新客户端回调给用户
typedef void (*NewConnectCB)(int clientid, void* userdata);

//TCPServer接收到客户端数据回调给用户
typedef void (*ServerRecvCB)(int clientid, const NetPacket& packethead, const unsigned char* buf, void* userdata);

//TCPClient接收到服务器数据回调给用户
typedef void (*ClientRecvCB)(const NetPacket& packethead, const unsigned char* buf, void* userdata);

//网络事件类型
typedef enum {
    NET_EVENT_TYPE_RECONNECT = 0,  //与服务器自动重连成功事件
    NET_EVENT_TYPE_DISCONNECT      //与服务器断开事件
} NET_EVENT_TYPE;
//TCPClient断线重连函数
typedef void (*ReconnectCB)(NET_EVENT_TYPE eventtype, void* userdata);

#endif//PACKET_SYNC_H