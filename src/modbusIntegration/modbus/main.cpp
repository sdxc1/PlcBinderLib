#include <iostream>
#include <chrono>
#include <functional>
#include "modeTcp.h"

// 异步回调打印函数，给modeTcp队列使用
void OnModbusResult(const CmdResult& res)
{
	std::cout << "\n=====【异步队列回调返回】=====" << std::endl;
	std::cout << "命令类型:" << res.type << " 地址:" << res.addr;
	if (res.success)
	{
		std::cout << " 状态:成功";
		if (res.type == CMD_READ_M || res.type == CMD_READ_D)
		{
			std::cout << " 整型值:" << res.valInt;
		}
		else if (res.type == CMD_READ_DFLOAT)
		{
			std::cout << " 浮点值:" << res.valFloat;
		}
	}
	else
	{
		std::cout << " 状态:失败";
	}
	std::cout << "\n==============================\n" << std::endl;
}

int main() {

	modeTcp* mTcp = new modeTcp();
	bool ret=false;
	if (0)
	{
		ret = mTcp->connectTcp("127.0.0.1", 502, 1, true);
	}
	else
	{
		ret = mTcp->connectRtu("\\\\.\\COM1", 9600, 'N', 8, 1,  true);
	}

	std::cout << "首次连接:" << (ret ? "TRUE" : "FALSE") << std::endl;
	if (mTcp->online()) {
		std::cout << "连接在线TRUE" << std::endl;
	}
	else {
		std::cout << "离线FALSE-开启线程" << std::endl;
	}

	// 注册异步回调
	mTcp->setCallback(OnModbusResult);

	int cmd;
	while (true) {
		std::cout << "\n==========请输入命令编号:" << "\n"
			<< "1=查询连接状态,2=读M地址,3=写M地址,4=读整D地址,5=写整D地址" << "\n"
			<< "6=读实D地址,7=写实D地址" << "\n"
			<< "8=批量读D整,9=批量写D整,10=批量读M,11=批量写M" << "\n"
			<< "12=批量读D浮点,13=批量写D浮点" << "\n"
			<< "14=启动消费线程,15=异步队列读M,16=连续压入10条队列压测,17=异步队列读D浮点" << "\n"
			<< "-1=退出" << std::endl;
		std::cin >> cmd;

		switch (cmd)
		{
			//==============================读在线
		case 1:
			if (mTcp->online())
				std::cout << "当前:在线" << std::endl;
			else
				std::cout << "当前:离线" << std::endl;
			break;

			//==============================读M
		case 2:
		{
			std::cout << "请输入M读取地址:" << std::endl;
			int M;
			uint8_t status;
			std::cin >> M;
			if (mTcp->readM(M, status)) {
				std::cout << "读取地址:" << M << "\n" << "当前状态:" << (int)status << std::endl;
			}
			else {
				std::cout << "读取M地址失败" << std::endl;
			}
			break;
		}

		//==============================写M
		case 3:
		{
			std::cout << "请输入M写入地址:" << std::endl;
			int MW;
			std::cin >> MW;
			std::cout << "请输入M写入布尔:" << std::endl;
			int VAL;
			std::cin >> VAL;
			if (mTcp->writeM(MW, VAL)) {
				std::cout << "写入地址:" << MW << "\n" << "写入数据:" << VAL << std::endl;
			}
			else {
				std::cout << "写入M地址失败" << std::endl;
			}
			break;
		}

		//==============================读D-整
		case 4:
		{
			std::cout << "请输入D读取地址:" << std::endl;
			int addr;
			int val;
			std::cin >> addr;
			if (mTcp->readD(addr, val)) {
				std::cout << "读取地址:" << addr << "\n" << "读取数据:" << (int)val << std::endl;
			}
			else {
				std::cout << "读取D地址失败" << std::endl;
			}
			break;
		}

		//==============================写D-整
		case 5:
		{
			std::cout << "请输入D写入地址:" << std::endl;
			int addrWD;
			std::cin >> addrWD;
			int valWD;
			std::cout << "请输入D写入数据:" << std::endl;
			std::cin >> valWD;
			if (mTcp->writeD(addrWD, valWD)) {
				std::cout << "写入地址:" << addrWD << "\n" << "写入数据:" << valWD << std::endl;
			}
			else {
				std::cout << "写入D地址失败" << std::endl;
			}
			break;
		}

		//==============================读D-实
		case 6:
		{
			std::cout << "请输入D读取地址:" << std::endl;
			int addrRD;
			std::cin >> addrRD;
			float valRD;
			std::cout << "请输入字节序mode(1=CDAB汇川,2=ABCD模拟器):" << std::endl;
			int modeRR;
			std::cin >> modeRR;
			if (mTcp->readDFloat(addrRD, valRD, modeRR))
			{
				std::cout << "读取地址:" << addrRD << "\n" << "读取浮点数:" << valRD << std::endl;
			}
			else
			{
				std::cout << "读取D浮点数失败" << std::endl;
			}
			break;
		}

		//==================================写D-实
		case 7:
		{
			std::cout << "请输入D写入地址:" << std::endl;
			int addrWR;
			std::cin >> addrWR;
			std::cout << "请输入要写入的浮点数:" << std::endl;
			float valWR;
			std::cin >> valWR;
			std::cout << "请输入字节序mode(1=CDAB汇川,2=ABCD模拟器):" << std::endl;
			int modeWR;
			std::cin >> modeWR;
			if (mTcp->writeDFloat(addrWR, valWR, modeWR))
			{
				std::cout << "写入地址:" << addrWR << " 写入浮点数:" << valWR << std::endl;
			}
			else
			{
				std::cout << "写入D浮点数失！" << std::endl;
			}
			break;
		}

		//==================================批量读D整
		case 8:
		{
			std::cout << "请输入D起始地址:" << std::endl;
			int addr;
			std::cin >> addr;
			std::cout << "请输入读取数量(最大125):" << std::endl;
			int cnt;
			std::cin >> cnt;

			uint16_t buf[125] = { 0 };
			if (mTcp->readMultiD(addr, cnt, buf))
			{
				std::cout << "批量读D成功!\n数据列表:" << std::endl;
				for (int i = 0; i < cnt; i++)
				{
					std::cout << "D" << (addr + i) << " = " << buf[i] << std::endl;
				}
			}
			else
			{
				std::cout << "批量读D整数失败!" << std::endl;
			}
			break;
		}

		//==================================批量写D整
		case 9:
		{
			std::cout << "请输入D起始地址:" << std::endl;
			int addr;
			std::cin >> addr;
			std::cout << "请输入写入数量(最大125):" << std::endl;
			int cnt;
			std::cin >> cnt;

			uint16_t buf[125] = { 0 };
			std::cout << "请依次输入" << cnt << "个整数:" << std::endl;
			for (int i = 0; i < cnt; i++)
			{
				std::cin >> buf[i];
			}

			if (mTcp->writeMultiD(addr, cnt, buf))
			{
				std::cout << "批量写D整数成功!" << std::endl;
			}
			else
			{
				std::cout << "批量写D整数失败!" << std::endl;
			}
			break;
		}

		//==================================批量读M
		case 10:
		{
			std::cout << "请输入M起始地址:" << std::endl;
			int addr;
			std::cin >> addr;
			std::cout << "请输入读取数量(最大2000):" << std::endl;
			int cnt;
			std::cin >> cnt;

			uint8_t buf[2000] = { 0 };
			if (mTcp->readMultiM(addr, cnt, buf))
			{
				std::cout << "批量读M成功!\n数据列表:" << std::endl;
				for (int i = 0; i < cnt; i++)
				{
					std::cout << "M" << (addr + i) << " = " << (int)buf[i] << std::endl;
				}
			}
			else
			{
				std::cout << "批量读M失败!" << std::endl;
			}
			break;
		}

		//==================================批量写M
		case 11:
		{
			std::cout << "请输入M起始地址:" << std::endl;
			int addr;
			std::cin >> addr;
			std::cout << "请输入写入数量(最大2000):" << std::endl;
			int cnt;
			std::cin >> cnt;

			uint8_t buf[2000] = { 0 };
			std::cout << "请依次输入" << cnt << "个0/1值:" << std::endl;
			for (int i = 0; i < cnt; i++)
			{
				std::cin >> buf[i];
			}

			if (mTcp->writeMultiM(addr, cnt, buf))
			{
				std::cout << "批量写M成功!" << std::endl;
			}
			else
			{
				std::cout << "批量写M失败!" << std::endl;
			}
			break;
		}

		//==================================批量读D浮点
		case 12:
		{
			std::cout << "请输入D浮点起始地址:" << std::endl;
			int addr;
			std::cin >> addr;
			std::cout << "请输入浮点数量(最大62):" << std::endl;
			int cnt;
			std::cin >> cnt;
			std::cout << "请输入字节序mode(1=CDAB汇川,2=ABCD模拟器):" << std::endl;
			int mode;
			std::cin >> mode;

			float buf[62] = { 0.0f };
			if (mTcp->readMultiDFloat(addr, cnt, buf, mode))
			{
				std::cout << "批量读浮点成功!\n数据列表:" << std::endl;
				for (int i = 0; i < cnt; i++)
				{
					std::cout << "D" << (addr + i * 2) << " = " << buf[i] << std::endl;
				}
			}
			else
			{
				std::cout << "批量读浮点失败!" << std::endl;
			}
			break;
		}

		//==================================批量写D浮点
		case 13:
		{
			std::cout << "请输入D浮点起始地址:" << std::endl;
			int addr;
			std::cin >> addr;
			std::cout << "请输入浮点数量(最大62):" << std::endl;
			int cnt;
			std::cin >> cnt;
			std::cout << "请输入字节序mode(1=CDAB汇川,2=ABCD模拟器):" << std::endl;
			int mode;
			std::cin >> mode;

			float buf[62] = { 0.0f };
			std::cout << "请依次输入" << cnt << "个浮点数:" << std::endl;
			for (int i = 0; i < cnt; i++)
			{
				std::cin >> buf[i];
			}

			if (mTcp->writeMultiDFloat(addr, cnt, buf, mode))
			{
				std::cout << "批量写浮点成功!" << std::endl;
			}
			else
			{
				std::cout << "批量写浮点失败!" << std::endl;
			}
			break;
		}
		//===============================14启动消费线程
		case 14:
		{
			mTcp->startConsume();
			std::cout << "消费线程已启动，可下发异步队列指令！" << std::endl;
			break;
		}
		//===============================15异步队列读M
		case 15:
		{
			std::cout << "请输入M读取地址:" << std::endl;
			int addr;
			std::cin >> addr;
			mTcp->addReadM(addr);
			std::cout << "异步读M命令已经压入队列，等待回调输出结果..." << std::endl;
			break;
		}
		//===============================16连续压入10条读M压测
		case 16:
		{
			std::cout << "连续压入M100~M109，共10条读M指令进队列" << std::endl;
			for (int i = 0; i < 10; i++)
			{
				mTcp->addReadM(100 + i);
			}
			std::cout << "10条命令全部入队，消费线程会串行依次执行，看回调输出" << std::endl;
			break;
		}
		//===============================17异步队列读D浮点
		case 17:
		{
			std::cout << "请输入D浮点起始地址:" << std::endl;
			int addr;
			std::cin >> addr;
			std::cout << "请输入字节序mode(1=CDAB汇川,2=ABCD模拟器):" << std::endl;
			int mode;
			std::cin >> mode;
			Cmd cmd;
			cmd.type = CMD_READ_DFLOAT;
			cmd.addr = addr;
			cmd.floatMode = mode;
			mTcp->addCmd(cmd);
			std::cout << "异步读D浮点命令压入队列，等待回调返回" << std::endl;
			break;
		}

		case -1:
			std::cout << "准备退出程序" << std::endl;
			goto exit_main;

		default:
			std::cout << "无效命令" << std::endl;
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}

exit_main:
	mTcp->stopConsume();
	delete mTcp;
	mTcp = nullptr;
	return 0;
}
