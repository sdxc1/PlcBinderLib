# PlcBinderLib

更简便地控制PLC。一行代码绑定UI控件与PLC地址，点击按钮自动读写，不用手写通信逻辑。

## 能干什么

- 原生Socket实现ModbusTCP通信，不依赖libmodbus等第三方库
- libmodbus后端，支持TCP和RTU双模式，批量读写，异步队列
- 一行代码把Button/TextBox绑定到PLC的M/D地址，点击自动执行读写
- 支持M区线圈(01/05)、D区整数(03/06)、D区浮点(03/10)
- 浮点读写支持5种字节序(汇川CDAB/模拟器ABCD等)
- 通信层和绑定层分离，切换后端只改一行代码

## 项目结构

```
PlcBinderLib/
├── src/
│   ├── cpp/QtPlcBinderLib/              C++ 静态库 + 测试程序
│   │   ├── QtPlcBinderLib/
│   │   │   ├── ICommBackend.h           抽象接口
│   │   │   ├── InovanceBackend.h        原生socket后端
│   │   │   ├── ModbusBackend.h          libmodbus后端
│   │   │   ├── InovanceModbusTcp.h/.cpp 原生socket通信实现
│   │   │   ├── qtBinde.h/.cpp           Qt UI绑定层
│   │   │   └── framework.h / pch.h     预编译头
│   │   ├── libTest/                     Qt测试程序
│   │   │   ├── main.cpp                 示例入口
│   │   │   └── libTest.h/.cpp           测试窗口
│   │   └── QtPlcBinderLib.sln
│   │
│   ├── modbusIntegration/              libmodbus通信库(独立可用)
│   │   ├── modbus/
│   │   │   ├── modeTcp.h/.cpp           TCP+RTU双模式,队列,批量操作
│   │   │   ├── main.cpp                 18项菜单测试程序
│   │   │   ├── include/                 libmodbus头文件
│   │   │   │   ├── modbus.h
│   │   │   │   ├── modbus-rtu.h
│   │   │   │   ├── modbus-tcp.h
│   │   │   │   └── ...
│   │   │   └── modbus.vcxproj
│   │   └── modbus.sln
│   │
│   └── cSharp/                         C# WinForm版
│       ├── InovancePlcTest.sln
│       ├── InovanceModbusTcp.cs        ModbusTCP通信-TcpClient实现
│       ├── ControlBinder.cs            WinForm UI绑定层
│       └── Form1.cs                    测试程序
│
└── README.md
```

## 架构

```
ICommBackend (抽象接口)
    |
    +-- InovanceBackend   原生socket, TCP, 不依赖第三方库
    +-- ModbusBackend     libmodbus, TCP+RTU, 通用设备
```

`qtBinde` 只认 `ICommBackend*` 指针，构造时传哪个后端就用哪个，绑定代码一行不改。

## 用法

### C++ / Qt

```cpp
// 选后端
ModbusBackend* backend = new ModbusBackend();
backend->setupTcp("127.0.0.1", 502, 1, true);  // IP 端口 站号 自动重连
backend->connect();

// 或者原生socket
// InovanceBackend* backend = new InovanceBackend();
// backend->setup("127.0.0.1", 502, 1);

// 绑定控件
qtBinde binder(backend);
binder.toggleM(btnM100, 100);               //点按钮切换M100
binder.writeD(btnWd, txtD100, 100);         //点按钮从输入框写D100
binder.readDF(btnRd, txtDF200, 200, 1);     //点按钮读D200浮点,1=汇川CDAB
```

### C# / WinForm

```csharp
var plc = new InovanceModbusTcp();
plc.Init("127.0.0.1", 502, 1);

var binder = new ControlBinder(plc);
binder.ToggleM(btnM100, 100);              //点按钮切换M100
binder.WriteD(btnWd, txtD100, 100);        //点按钮从输入框写D100
binder.ReadDF(btnRd, txtDF200, 200);       //点按钮读D200浮点显示
```

## 绑定方法一览

| 方法    | 功能                            | 功能码    |
| ------- | ------------------------------- | --------- |
| ToggleM | 点击切换M线圈(读当前值取反写回) | 01读/05写 |
| ReadM   | 点击读M显示ON/OFF               | 01        |
| WriteD  | 点击从输入框写D整数             | 06        |
| ReadD   | 点击读D整数显示                 | 03        |
| WriteDF | 点击从输入框写D浮点             | 10        |
| ReadDF  | 点击读D浮点显示                 | 03        |

## 两个后端怎么选

|            | InovanceBackend       | ModbusBackend         |
| ---------- | --------------------- | --------------------- |
| 传输方式   | TCP (原生winsock)     | TCP + RTU (libmodbus) |
| 单个读写   | M线圈 / D整数 / D浮点 | M线圈 / D整数 / D浮点 |
| 批量读写   | 不支持                | D整数 / M / D浮点     |
| 异步队列   | 不支持                | 支持,去重+上限500     |
| 自动重连   | 有                    | 有                    |
| 浮点字节序 | 固定CDAB              | 5种可选               |
| 依赖       | winsock (系统自带)    | libmodbus             |

简单场景用 InovanceBackend，不依赖第三方库。需要批量读写或 RTU 时用 ModbusBackend。

## 浮点字节序

不同 PLC 厂商的浮点存储顺序不同，ModbusBackend 支持 5 种：

| mode | 读取 | 写入 | 适用设备          |
| ---- | ---- | ---- | ----------------- |
| 0    | 默认 | 默认 | libmodbus默认     |
| 1    | CDAB | DCBA | 汇川PLC           |
| 2    | ABCD | BADC | ModbusSlave模拟器 |
| 3    | BADC | ABCD | 预留              |
| 4    | DCBA | CDAB | 预留              |

libmodbus 的 set/get_float 系列函数命名与实际字节排列相反，读写需要对调使用。

## 异步队列

只有 ModbusBackend 有。适合高频读写场景，比如界面轮询。

```cpp
ModbusBackend* backend = new ModbusBackend();
backend->setupTcp("127.0.0.1", 502, 1, true);
backend->connect();
backend->setCallback([](const CmdResult& res) {
    // 回调里拿结果
});
backend->startConsume();

// 投递命令,不阻塞
backend->addReadM(0);
backend->addReadD(100);
backend->addReadDFloat(200, 1);
```

队列内部串行执行，同类同地址自动去重，满500条时读操作丢弃写操作保留。一个 PLC 一个实例一个消费线程，不用加锁。

## 构建

C++版需要 Visual Studio 2022 + Qt，打开 `src/cpp/QtPlcBinderLib/QtPlcBinderLib.sln` 编译。

C#版需要 .NET Framework 4.8，打开 `src/cSharp/InovancePlcTest.sln` 或 `dotnet build`。

modbusIntegration 独立编译，打开 `src/modbusIntegration/modbus.sln`，需要 libmodbus。

## License

MIT
