#include "modeTcp.h"

modeTcp::modeTcp() = default;

modeTcp::~modeTcp()
{
    close();
}

//==============================通用基础策略
//===============连接与断开
bool modeTcp::connectTcp(const char* ip, int port, int station, bool reconnect)
{
    //清理旧连接
    close();
    //映射地址
    ipTcp = ip;
    portTcp = port;
    stationTcp = station;
    //标记TCP模式
    linkType = LINK_TCP;
    //若开重连线程-优先开启线程防止失败直接弹出
    if (reconnect) {
        isReconnect = true;
        std::thread(&modeTcp::reconnectLoop, this).detach();
    }
    //先连一次
    if (!tryConnect()) return false;
    return true;
}

//===============RTU连接
bool modeTcp::connectRtu(const char* port, int baud, char parity,
    int dataBits, int stopBits, bool reconnect)
{
    //清理旧连接
    close();
    //映射串口参数
    portRtu = port;
    baudRtu = baud;
    parityRtu = parity;
    dataBitsRtu = dataBits;
    stopBitsRtu = stopBits;
    //标记RTU模式
    linkType = LINK_RTU;
    if (reconnect) {
        isReconnect = true;
        std::thread(&modeTcp::reconnectLoop, this).detach();
    }
    if (!tryConnect()) return false;
    return true;
}

bool modeTcp::tryConnect()
{
    //手持锁才可以尝试连接
    std::lock_guard<std::mutex> lk(lockConnection);

    //按链接类型创建上下文
    if (linkType == LINK_TCP) {
        //=====TCP
        mModbus = modbus_new_tcp(ipTcp.c_str(), portTcp);
    }
    else {
        //=====RTU
        mModbus = modbus_new_rtu(portRtu.c_str(), baudRtu, parityRtu,
            dataBitsRtu, stopBitsRtu);
    }

    if (!mModbus) { onlineModbus = false; return false; }
    //设置站号
    modbus_set_slave(mModbus, stationTcp);
    //500ms超时
    modbus_set_response_timeout(mModbus, 0, 500000);
    //尝试连接
    if (modbus_connect(mModbus) == -1) {
        modbus_free(mModbus);
        mModbus = nullptr;
        onlineModbus = false;
        return false;
    }
    //成功返回
    onlineModbus = true;
    return true;
}

void modeTcp::reconnectLoop()
{
    //开启线程
    while (isReconnect) {
        if (!onlineModbus) tryConnect();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void modeTcp::close()
{
    //清空复位
    isReconnect = false;
    std::lock_guard<std::mutex> lk(lockConnection);
    if (mModbus) {
        modbus_close(mModbus);
        modbus_free(mModbus);
        mModbus = nullptr;
    }
    onlineModbus = false;
}

//=====单一操作
//===============M操作===============
//===============M读
bool modeTcp::readM(int addr, uint8_t& out)
{
    if (!onlineModbus || !mModbus) {
        return false;
    }
    //数据锁-保证真正与数据互动
    std::lock_guard<std::mutex> lk(lockConnection);
    uint8_t val = 0;
    if (modbus_read_bits(mModbus, addr, 1, &val)==-1) {
        //失败清空连接标志
        onlineModbus = false;
        return false;
    }
    //数据弹出
    out = val;
    return true;
}
//===============M写
bool modeTcp::writeM(int addr, int val)
{
    if (!onlineModbus || !mModbus) {
        return false;
    }
    std::lock_guard<std::mutex> lk(lockConnection);
    if (val > 1) {
        return false;
    }
    if (modbus_write_bit(mModbus, addr, val)==-1) {
        onlineModbus = false;
        return false;
    }
    return true;
}

//===============D整型操作===============
//===============D读整
bool modeTcp::readD(int addr, int& out)
{
    if (!onlineModbus || !mModbus) {
        return false;
    }
    std::lock_guard<std::mutex> lk(lockConnection);
    //临时承接变量
    uint16_t temp = 0;
    if (modbus_read_registers(mModbus, addr, 1, &temp) == -1) {
        onlineModbus = false;
        return false;
    }
    out = temp;
    return true;
}
//===============D写整
bool modeTcp::writeD(int addr ,int val) {
    if (!onlineModbus || !mModbus) {
        return false;
    }
    std::lock_guard<std::mutex> lk(lockConnection);
    //数据溢出自动弹出
    if (val < 0 || val > 65535) {
        return false;
    }
    if (modbus_write_register(mModbus, addr, val) == -1) {
        onlineModbus = false;
        return false;
    }
    return true;
}

//===============D实数操作===============(lib版本读写倒叙-注意)
//===============D读实
bool modeTcp::readDFloat(int addr, float& out,int mode) {
    if (!onlineModbus || !mModbus) {
        return false;
    }
    std::lock_guard<std::mutex> lk(lockConnection);
    uint16_t temp[2] = { 0 };
    int ret = modbus_read_registers(mModbus, addr, 2, temp);
    if (ret == -1 || ret != 2) {
        onlineModbus = false;
        return false;
    }
    switch (mode) {
        //默认
    case 0: out = modbus_get_float(temp); break;
        //汇川PLC
    case 1: out = modbus_get_float_cdab(temp); break;
        //ModbusSlave
    case 2: out = modbus_get_float_abcd(temp); break;
        //待添加
    case 3: out = modbus_get_float_badc(temp); break;
        //待添加
    case 4: out = modbus_get_float_dcba(temp); break;
        //非正规模式
    default: return false;
    }
    return true;
}
//===============D写实
bool modeTcp::writeDFloat(int addr, float val, int mode)
{
    if (!onlineModbus || !mModbus) {
        return false;
    }
    std::lock_guard<std::mutex> lk(lockConnection);
    uint16_t temp[2] = { 0 };

    switch (mode) {
        //默认
    case 0: modbus_set_float(val, temp); break;
        //汇川PLC
    case 1: modbus_set_float_dcba(val, temp); break;
        //ModbusSlave
    case 2: modbus_set_float_badc(val, temp); break;
        //待添加
    case 3: modbus_set_float_abcd(val, temp); break;
        //待添加
    case 4: modbus_set_float_cdab(val, temp); break;
        //非正规模式
    default: return false;
    }

    int ret = modbus_write_registers(mModbus, addr, 2, temp);
    if (ret == -1 || ret != 2) {
        onlineModbus = false;
        return false;
    }
    return true;
}

//=====批量操作
//======================批量读D整
bool modeTcp::readMultiD(int addr, int count, uint16_t* buf)
{
    if (!onlineModbus || !mModbus)
    {
        onlineModbus = false;
        return false;
    }
    //D寄存器批量最多125个
    if (count <= 0 || count > 125 || !buf) return false;
    std::lock_guard<std::mutex> lock(lockConnection);
    if (modbus_read_registers(mModbus, addr, count, buf) == -1)
    {
        onlineModbus = false;
        return false;
    }
    return true;
}
// ======================批量写D整
bool modeTcp::writeMultiD(int addr, int count, const uint16_t* buf)
{
    if (!onlineModbus || !mModbus)
    {
        onlineModbus = false;
        return false;
    }
    if (count <= 0 || count > 125 || !buf) return false;
    std::lock_guard<std::mutex> lock(lockConnection);
    if (modbus_write_registers(mModbus, addr, count, buf) == -1)
    {
        onlineModbus = false;
        return false;
    }
    return true;
}

// ======================批量读M
bool modeTcp::readMultiM(int addr, int count, uint8_t* buf)
{
    if (!onlineModbus || !mModbus)
    {
        onlineModbus = false;
        return false;
    }
    //M寄存器最大2000批量
    if (count <= 0 || count > 2000 || !buf) return false;
    std::lock_guard<std::mutex> lock(lockConnection);
    if (modbus_read_bits(mModbus, addr, count, buf) == -1)
    {
        onlineModbus = false;
        return false;
    }
    return true;
}
// ======================批量写M
bool modeTcp::writeMultiM(int addr, int count, const uint8_t* val)
{
    if (!onlineModbus || !mModbus)
    {
        onlineModbus = false;
        return false;
    }
    if (count <= 0 || count > 2000 || !val) return false;
    std::lock_guard<std::mutex> lock(lockConnection);
    if (modbus_write_bits(mModbus, addr, count, val) == -1)
    {
        onlineModbus = false;
        return false;
    }
    return true;
}

// ======================批量读D实
bool modeTcp::readMultiDFloat(int addr, int count, float* out, int mode)
{
    if (!onlineModbus || !mModbus)
    {
        onlineModbus = false;
        return false;
    }
    //float-1个占2D,最多62个float-(62*2=124D<=125)
    if (count <= 0 || count > 62 || !out) return false;
    std::lock_guard<std::mutex> lock(lockConnection);
    //动态数组扩大容器与清理
    std::vector<uint16_t> buf(count * 2, 0);
    if (modbus_read_registers(mModbus, addr, count * 2, buf.data()) == -1)
    {
        onlineModbus = false;
        return false;
    }
    //遍历读取
    for (int i = 0; i < count; ++i)
    {
        const uint16_t* pReg = &buf[i * 2];
        switch (mode)
        {
        case 0: out[i] = modbus_get_float(pReg); break;//默认
        case 1: out[i] = modbus_get_float_dcba(pReg); break;//汇川
        case 2: out[i] = modbus_get_float_badc(pReg); break;//ModbusSlave
        case 3: out[i] = modbus_get_float_abcd(pReg); break;
        case 4: out[i] = modbus_get_float_cdab(pReg); break;
        default: return false;
        }
    }
    return true;
}
// ======================批量写D实
bool modeTcp::writeMultiDFloat(int addr, int count, const float* vals, int mode)
{
    if (!onlineModbus || !mModbus)
    {
        onlineModbus = false;
        return false;
    }
    if (count <= 0 || count > 62 || !vals) return false;
    std::lock_guard<std::mutex> lock(lockConnection);
    std::vector<uint16_t> buf(count * 2, 0);
    for (int i = 0; i < count; ++i)
    {
        uint16_t* pReg = &buf[i * 2];
        switch (mode)
        {
        case 0: modbus_set_float(vals[i], pReg); break;//默认
        case 1: modbus_set_float_dcba(vals[i], pReg); break;//汇川
        case 2: modbus_set_float_badc(vals[i], pReg); break;//ModbusSlave
        case 3: modbus_set_float_abcd(vals[i], pReg); break;
        case 4: modbus_set_float_cdab(vals[i], pReg); break;
        default: return false;
        }
    }
    if (modbus_write_registers(mModbus, addr, 2 * count, buf.data()) == -1)
    {
        onlineModbus = false;
        return false;
    }
    return true;
}

//==============================队列消费策略
//===============启动消费线程
void modeTcp::startConsume()
{
    if (isRunning) return;
    isRunning = true;
    consumerThread = std::thread(&modeTcp::consumeLoop, this);
}

//===============停止消费线程
void modeTcp::stopConsume()
{
    isRunning = false;
    cvQueue.notify_all();          //唤醒等待中的消费线程
    if (consumerThread.joinable())
        consumerThread.join();     //等待线程结束
}

//===============设置结果回调
void modeTcp::setCallback(std::function<void(const CmdResult&)> cb)
{
    callbackResult = std::move(cb);
}

//===============生产-投递命令到队列-去重+上限策略
void modeTcp::addCmd(const Cmd& cmd)
{
    std::lock_guard<std::mutex> lk(lockQueue);

    //写操作去重-同地址同类型只留最新一条
    if (cmd.type == CMD_WRITE_M ||
        cmd.type == CMD_WRITE_D ||
        cmd.type == CMD_WRITE_DFLOAT)
    {
        std::queue<Cmd> temp;
        while (!cmdQueue.empty()) {
            Cmd& front = cmdQueue.front();
            //同类型同地址的旧写命令丢弃
            if (front.type == cmd.type && front.addr == cmd.addr) {
                cmdQueue.pop();
                continue;
            }
            temp.push(std::move(front));
            cmdQueue.pop();
        }
        cmdQueue = std::move(temp);
    }

    //读操作去重-同地址同类型只留最新一条
    if (cmd.type == CMD_READ_M ||
        cmd.type == CMD_READ_D ||
        cmd.type == CMD_READ_DFLOAT)
    {
        std::queue<Cmd> temp;
        while (!cmdQueue.empty()) {
            Cmd& front = cmdQueue.front();
            if (front.type == cmd.type && front.addr == cmd.addr) {
                cmdQueue.pop();
                continue;
            }
            temp.push(std::move(front));
            cmdQueue.pop();
        }
        cmdQueue = std::move(temp);
    }

    //队列上限500-满时读丢弃-写强制塞入
    if ((int)cmdQueue.size() >= 500) {
        //读操作直接丢弃
        if (cmd.type == CMD_READ_M ||
            cmd.type == CMD_READ_D ||
            cmd.type == CMD_READ_DFLOAT)
        {
            return;
        }
        //写操作强制塞入
    }
    cmdQueue.push(cmd);
    //唤醒消费线程
    cvQueue.notify_one();
}

//===============快捷投递-读M
void modeTcp::addReadM(int addr)
{
    Cmd c;
    c.type = CMD_READ_M; c.addr = addr;
    c.valInt = 0; c.valFloat = 0; c.floatMode = 0; c.count = 0;
    addCmd(c);
}

//===============快捷投递-写M
void modeTcp::addWriteM(int addr, int val)
{
    Cmd c;
    c.type = CMD_WRITE_M; c.addr = addr;
    c.valInt = val; c.valFloat = 0; c.floatMode = 0; c.count = 0;
    addCmd(c);
}

//===============快捷投递-读D整
void modeTcp::addReadD(int addr)
{
    Cmd c;
    c.type = CMD_READ_D; c.addr = addr;
    c.valInt = 0; c.valFloat = 0; c.floatMode = 0; c.count = 0;
    addCmd(c);
}

//===============快捷投递-写D整
void modeTcp::addWriteD(int addr, int val)
{
    Cmd c;
    c.type = CMD_WRITE_D; c.addr = addr;
    c.valInt = val; c.valFloat = 0; c.floatMode = 0; c.count = 0;
    addCmd(c);
}

//===============快捷投递-读D实
void modeTcp::addReadDFloat(int addr, int mode)
{
    Cmd c;
    c.type = CMD_READ_DFLOAT; c.addr = addr;
    c.valInt = 0; c.valFloat = 0; c.floatMode = mode; c.count = 0;
    addCmd(c);
}

//===============快捷投递-写D实
void modeTcp::addWriteDFloat(int addr, float val, int mode)
{
    Cmd c;
    c.type = CMD_WRITE_DFLOAT; c.addr = addr;
    c.valInt = 0; c.valFloat = val; c.floatMode = mode; c.count = 0;
    addCmd(c);
}

//===============消费线程-循环处理队列
void modeTcp::consumeLoop()
{
    while (isRunning) {
        Cmd cmd;
        {
            //加锁取命令,取完解锁
            std::unique_lock<std::mutex> lk(lockQueue);
            //队列为空时挂起等待,有命令或要退出时被唤醒
            cvQueue.wait(lk, [&] {
                return !cmdQueue.empty() || !isRunning;
                });
            if (!isRunning) break;
            cmd = cmdQueue.front();
            cmdQueue.pop();
        }

        //执行命令
        CmdResult result;
        result.addr = cmd.addr;
        result.type = cmd.type;
        result.success = false;
        result.valInt = 0;
        result.valFloat = 0;

        switch (cmd.type) {
        case CMD_READ_M: {
            unsigned char v = 0;
            result.success = readM(cmd.addr, v);
            result.valInt = v;
            break;
        }
        case CMD_WRITE_M: {
            result.success = writeM(cmd.addr, cmd.valInt);
            break;
        }
        case CMD_READ_D: {
            int v = 0;
            result.success = readD(cmd.addr, v);
            result.valInt = v;
            break;
        }
        case CMD_WRITE_D: {
            result.success = writeD(cmd.addr, cmd.valInt);
            break;
        }
        case CMD_READ_DFLOAT: {
            float v = 0;
            result.success = readDFloat(cmd.addr, v, cmd.floatMode);
            result.valFloat = v;
            break;
        }
        case CMD_WRITE_DFLOAT: {
            result.success = writeDFloat(cmd.addr, cmd.valFloat, cmd.floatMode);
            break;
        }
        default: break;
        }

        //回调通知结果
        if (callbackResult) {
            callbackResult(result);
        }
    }
}