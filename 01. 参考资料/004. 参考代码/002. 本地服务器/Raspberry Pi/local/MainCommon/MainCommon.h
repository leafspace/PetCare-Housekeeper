#pragma once

#define DRIVESIZE   20
#define LDV7SPEED   9600

#include "../ComCommon/ComCommon.h"
#include "../XMLMessage/ASRQueue/ASRQueue.h"
#include "../Thread/Listener/ThreadListerner.h"

/*
***********************************************************
*
*	函数名	: printUsefulDriveList
*	功能	: 打印范围内设备可用状态
*	参数	:
				【in】driveSize  : 设备号范围
				【in】driveID    : 设备号保存处
*	返回值	: 无
*
***********************************************************
*/
void printUsefulDriveList(int driveSize, int *driveID);

/*
***********************************************************
*
*	函数名	: chooseUsefulDrive
*	功能	: 用户选择一个可用的设备
*	参数	:
				【in】driveSize  : 设备号范围
				【in】driveID    : 设备号保存处
*	返回值	: 【ret】comFId      : 可用设备号
*
***********************************************************
*/
int chooseUsefulDrive(int driveSize, int *driveID);

/*
***********************************************************
*
*	函数名	: setDriveParam
*	功能	: 设置设备的波特率等参数
*	参数	:
				【in】comID  : 设备号
				【in】speed  : 设备波特率
				【in】dataBits : 设备一次发送的比特数
				【in】stopBits : 设备号保存处
				【in】parity : 设备号保存处
*	返回值	: 【ret】 isSuccess : 设置成功与否
*
***********************************************************
*/
bool setDriveParam(int comID, int speed, int dataBits, int stopBits, int parity);

/*
***********************************************************
*
*	函数名	: sendRequest
*	功能	: 向定义的目标主机发送get请求
*	参数	:
				【in】requestWay   : 用户使用的请求方式：例如GET或POST
				【in】contentType   : 用户要请求的类型：例如text/html
				【in】keepRecive   : 用户是否要在发送结束后继续监听返回的数据
*	返回值	: 【ret】isSuccess : 是否成功
*
***********************************************************
*/
bool sendRequest(char *requestWay, char *contentType, bool keepRecive);

/*
***********************************************************
*
*	函数名	: SplitASRKeyWord
*	功能	: 将字符串转为识别的关键词码队列
*	参数	:
				【in】message   : 单片机发过来的识别电文串
				【out】queue   : 返回的关键词码队列
*	返回值	: 【ret】isSuccess : 是否成功
*
***********************************************************
*/
bool SplitASRKeyWord(char* message, ASRQueue *queue);