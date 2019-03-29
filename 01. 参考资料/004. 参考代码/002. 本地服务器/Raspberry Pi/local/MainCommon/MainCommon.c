#include "MainCommon.h"

/*
***********************************************************
*
*	函数名	: printUsefulDriveList
*	功能	: 打印范围内设备可用状态
*	参数	:
				【in】driveSize    : 设备号范围
				【in】driveID    : 设备号保存处
*	返回值	: 无
*
***********************************************************
*/
void printUsefulDriveList(int driveSize, int *driveID)
{
	printf("TIP : Com drive list :\n");
	int i = 0;
	char dirveName[15] = { 0 };
	for (i = 0; i < DRIVESIZE; ++i) {
		sprintf(dirveName, "/dev/ttyUSB%d", i);
		printf("    NO.%d\t%d\t%s\t", (i + 1), driveID[i], dirveName);
		if (driveID[i] < 0) {
			printf("NG\n");
		}
		else {
			printf("OK\n");
		}
	}
}

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
int chooseUsefulDrive(int driveSize, int *driveID)
{
	printf("TIP : Witch one drive you want select : ");
	do {
		int comFId = 0;
		char dirveName[15] = { 0 };
		scanf("%d", &comFId);
		if (comFId <= 0 || comFId > DRIVESIZE) {
			printf("ERROR : Wrong dirve number !\n");
			printf("TIP : Please enter a new drive number :");
		}
		else if (driveID[comFId - 1] < 0) {
			printf("ERROR : Driver can't use !\n");
			printf("TIP : Please enter a new drive number :");
		}
		else {
			sprintf(dirveName, "/dev/ttyUSB%d", comFId - 1);
			printf("TIP : You choose [%d] dirver (%s) .\n", comFId - 1, dirveName);
			return comFId - 1;
		}
	} while (true);
}

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
bool setDriveParam(int comID, int speed, int dataBits, int stopBits, int parity)
{
	bool isSuccess = false;
	printf("TIP : ComID = %d, Speed = %d . \n", comID, speed);
	isSuccess = set_speed(comID, speed);
	if (isSuccess == false) {
		printf("ERROR : Set speed failed ! \n");
		return false;
	}

	printf("TIP : ComID = %d, DataBits = %d, StopBits = %d, Parity = %c . \n", comID, dataBits, stopBits, parity);
	isSuccess = set_parity(comID, dataBits, stopBits, parity);
	if (isSuccess == false) {
		printf("ERROR : Set parity failed ! \n");
		return false;
	}
	return isSuccess;
}


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
bool sendRequest(char *requestWay, char *contentType, bool keepRecive)
{
	bool isSuccess = true;
	cancelWebListerner();
	initWebAddress();
	isSuccess = doRequest(requestWay, contentType, keepRecive);
	if (isSuccess == false) {
		printf("ERROR : Send <GET> request failed !\n");
		return false;
	}

	isSuccess = createWebListener(true);
	if (isSuccess == false) {
		printf("ERROR : Create web listener thread failed !\n");
		return false;
	}

	return true;
}

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
bool SplitASRKeyWord(char* message, ASRQueue *queue)
{
	int i = 0, keyNum = 0;
	char *keyWordBeginPosition = NULL, *keyWordEndPosition = NULL;
	keyWordBeginPosition = strstr(message, "<LDV7 REG>") + strlen("<LDV7 REG>");
	keyWordEndPosition = strstr(message, "</LDV7 REG>");
	for (i = 0; (keyWordBeginPosition + i) < keyWordEndPosition; ++i) {
		if (*(keyWordBeginPosition + i) == ' ') {
			pushASRQueue(queue, keyNum);
			keyNum = 0;
		}
		else {
			keyNum = keyNum * 10 + *(keyWordBeginPosition + i) - '0';
		}
	}
	pushASRQueue(queue, keyNum);
	return true;
}