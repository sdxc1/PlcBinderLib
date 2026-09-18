#pragma once
#include <modbus.h>
#include <thread>
#include <string>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <functional>
#include <vector>
#include <atomic>
#include <cstring>

//===============链接类型
enum LinkType {
    LINK_TCP,       //TCP
    LINK_RTU        //RTU串口
};

//===============命令类型
enum CmdType {
    CMD_READ_M,           //读M
    CMD_WRITE_M,          //写M
    CMD_READ_D,           //读D整
    CMD_WRITE_D,          //写D整
    CMD_READ_DFLOAT,      //读D实
    CMD_WRITE_DFLOAT,     //写D实
    CMD_READ_MULTI_D,     //批量读D整
    CMD_WRITE_MULTI_D,    //批量写D整
    CMD_READ_MULTI_M,     //批量读M
    CMD_WRITE_MULTI_M     //批量写M
};

//===============命令结构体
struct Cmd {
    CmdType type;           //命令类型
    int addr;               //地址
    int valInt;             //整型值
    float valFloat;         //浮点值
    int floatMode;          //浮点字节序(0默认,1汇川,2模拟器,3,4)
    int count;                  //批量数量
    uint16_t* buf16 = nullptr;   //批量D缓冲区
    uint8_t* buf8 = nullptr;     //批量M缓冲区
    float* bufFloat = nullptr;   //批量浮点缓冲区
};

//===============结果结构体
struct CmdResult {
    CmdType type;           //命令类型
    int addr;               //地址
    bool success;           //成功失败
    int valInt;             //读到的整型值
    float valFloat;         //读到的浮点值
};

class modeTcp
{
public:
    modeTcp();
    ~modeTcp();

    //开启连接-TCP
    bool connectTcp(const char* ip, int port = 502, int station = 1, bool reconnect = false);
    //开启连接-RTU
    bool connectRtu(const char* port, int baud = 9600, char parity = 'N',
        int dataBits = 8, int stopBits = 1, bool reconnect = false);

    //关闭连接
    void close();
    //在线状态
    bool online() const { return onlineModbus; }

    //调试
    modbus_t* ctx() { return mModbus; }

    //===============操作区
    //=====单一操作
    //读M
    bool readM(int addr, uint8_t& out);
    //写M
    bool writeM(int addr, int val);
    //读D-整数
    bool readD(int addr , int& out);
    //写D-整数
    bool writeD(int addr, int val);
    //读D-实数-模式:
    //1-汇川 2-ModbusSlave
    bool readDFloat(int addr, float& out,int mode);
    //写D-实数-模式:
    //1-汇川 2-ModbusSlave,(此版本倒叙写入例如abcd读取,badc写入)
    bool writeDFloat(int addr, float out,int mode);

    //=====批量操作
    //批量读D整
    bool readMultiD(int addr, int count, uint16_t* buf);
    //批量写D整
    bool writeMultiD(int addr, int count, const uint16_t* buf);
    //批量读M
    bool readMultiM(int addr, int count, uint8_t* buf);
    //批量写M
    bool writeMultiM(int addr, int count, const uint8_t* val);
    //批量读D实
    //1-汇川 2-ModbusSlave
    bool readMultiDFloat(int addr, int count, float* out, int mode);
    //批量写D实数
    //1-汇川 2-ModbusSlave,(此版本倒叙写入例如abcd读取,badc写入)
    bool writeMultiDFloat(int addr, int count, const float* vals, int mode);

    //===============生产消费
    void startConsume();                                        //启动消费线程
    void stopConsume();                                         //停止消费线程
    void setCallback(std::function<void(const CmdResult&)> cb); //设置结果回调
    void addCmd(const Cmd& cmd);                                //投递命令
    void addReadM(int addr);                                    //投递-读M
    void addWriteM(int addr, int val);                          //投递-写M
    void addReadD(int addr);                                    //投递-读D整
    void addWriteD(int addr, int val);                          //投递-写D整
    void addReadDFloat(int addr, int mode = 0);                 //投递-读D实
    void addWriteDFloat(int addr, float val, int mode = 0);     //投递-写D实



private:
    bool tryConnect();           //尝试连接
    void reconnectLoop();        //重连线程
    void consumeLoop();          //消费循环
    //指针结构体
    modbus_t* mModbus = nullptr;
    //在线状态
    std::atomic<bool> onlineModbus{ false };
    //重连使能
    std::atomic<bool> isReconnect{ false };
    
    //连接锁
    std::mutex lockConnection;

    //地址-TCP
    std::string ipTcp = "127.0.0.1";
    int portTcp = 502;
    int stationTcp = 1;

    //地址-RTU
    std::string portRtu = "COM1";
    int baudRtu = 9600;
    char parityRtu = 'N';
    int dataBitsRtu = 8;
    int stopBitsRtu = 1;

    //链接类型初始化
    LinkType linkType = LINK_TCP;

    //队列
    std::queue<Cmd> cmdQueue;                             //命令队列
    std::mutex lockQueue;                                 //队列锁
    std::condition_variable cvQueue;                      //条件变量
    std::atomic<bool> isRunning{ false };                 //消费线程运行标志
    std::thread consumerThread;                           //消费线程
    std::function<void(const CmdResult&)> callbackResult; //结果回调
};