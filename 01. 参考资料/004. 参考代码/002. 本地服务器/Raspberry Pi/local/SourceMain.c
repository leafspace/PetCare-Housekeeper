#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include "AllCommon/Common.h"
#include "ComCommon/ComCommon.h"
#include "MainCommon/MainCommon.h"
#include "Thread/Listener/ThreadListerner.h"
#include "WebServer/ServerCommon/ServerCommon.h"
#include "XMLMessage/MakeXMLMessage/MakeXMLMessage.h"

int main()
{
	do {
		// 查找可用设备
		int usefulDriveNum = 0;
		char dirveName[15] = { 0 };
		int driveID[DRIVESIZE] = { -1 };

		usefulDriveNum = findUsefulDriveList(DRIVESIZE, driveID);

		// 打印设备可用状态
		printUsefulDriveList(DRIVESIZE, driveID);

		// 没有可以使用的设备
		int comFId = 0;
		if (usefulDriveNum == 0) {
			printf("ERROR : Have no com drive !\n");
			break;
		}
		else if (usefulDriveNum == 1) {
			printf("TIP : Only have one drive .\n");
			int i = 0;
			for (i = 0; i < DRIVESIZE; ++i) {
				if (driveID[i] >= 0) {
					comFId = driveID[i];
					sprintf(dirveName, "/dev/ttyUSB%d", i);
					printf("TIP : You choose [%d] dirver (%s) .\n", i + 1, dirveName);
					break;
				}
			}
		}
		else {
			// 用户选择一个可用的设备
			comFId = chooseUsefulDrive(DRIVESIZE, driveID);
		}

		// 设置波特率参数
		bool isSuccess = setDriveParam(comFId, LDV7SPEED, 8, 1, 'N');
		if (isSuccess == false) {
			printf("ERROR : Can't set dirver[%d]'s parity !\n", comFId);
			continue;
		}

		// 读取消息
		char *infoBuffer = malloc(sizeof(char)* BUFFERSIZE);
		if (infoBuffer == NULL) {
			printf("ERROR : Not enough memory !\n");
		}

		// 为web服务器创建一个线程
		isSuccess = createWebListener(true);
		if (isSuccess == false) {
			printf("ERROR : Create web listener thread failed !\n");
			break;
		}

		// 不停监听来自单片机的消息并处理
		int readSize = -1;
		printf("TIP : [LDV7] Info list . Waiting ... \n");
		do {
			if ((readSize = read(comFId, infoBuffer, BUFFERSIZE)) > 0) {
				infoBuffer[readSize] = 0;
				printf("[LDV7] : %s", infoBuffer);
				if (strstr(infoBuffer, "<LDV7 REG>") && strstr(infoBuffer, "</LDV7 REG>")) {
					// 解析识别的识别码列表为队列的形式
					ASRQueue asrQueue;
					initASRQueue(&asrQueue);
					isSuccess = SplitASRKeyWord(infoBuffer, &asrQueue);
					if (isSuccess == false) {
						printf("ERROR : Resolve the identifier list failed ! \n");
						continue;
					}

					// 按照识别的识别码列表内容制作电文
					// 存放入../web/web/xml/UserMake文件夹中
					isSuccess = makeXMLMessage(&asrQueue);
					if (isSuccess == false) {
						printf("ERROR : Make message failed ! \n");
						continue;
					}

					// 向MFC发送请求
					isSuccess = sendRequest(REQUEST_GET, CONTENT_TYPE_HTML, false);
					if (isSuccess == false) {
						break;
					}
				}
			}
			else {
				break;
			}
		} while (true);
		printf("TIP : Finish link . \n");

		free(infoBuffer);
		closeUsefulDriveList(DRIVESIZE, driveID);
	} while (true);
	return 0;
}
