#include "../MainCommon/MainCommon.h"
#include "../Thread/Listener/ThreadListerner.h"
#include "../XMLMessage/MakeXMLMessage/MakeXMLMessage.h"

int main()
{
	int isSuccess = 0;

	/*
		char infoBuffer[BUFFERSIZE] = { 0 };
		strcpy(infoBuffer, "<LDV7 REG>4</LDV7 REG>");
		if (strstr(infoBuffer, "<LDV7 REG>") && strstr(infoBuffer, "</LDV7 REG>")) {
			// 解析识别的识别码列表为队列的形式
			ASRQueue asrQueue;
			initASRQueue(&asrQueue);
			isSuccess = SplitASRKeyWord(infoBuffer, &asrQueue);
			if (isSuccess == false) {
				printf("ERROR : Resolve the identifier list failed ! \n");
			}

			// 按照识别的识别码列表内容制作电文
			// 存放入../web/web/xml/UserMake文件夹中
			isSuccess = makeXMLMessage(&asrQueue);
			if (isSuccess == false) {
				printf("ERROR : Make message failed ! \n");
			}
		}
	*/

	isSuccess = createWebListener(true);
	if (isSuccess == false) {
		printf("ERROR : Create web listener thread failed !\n");
	}
	/*
		// 睡眠20秒，用来设置参数
		sleep(20);

		cancelWebListerner();
		initWebAddress();
		isSuccess = doRequest(REQUEST_GET, CONTENT_TYPE_HTML, false);
		if (isSuccess == false) {
			printf("ERROR : Send <GET> request failed !\n");
		}

		isSuccess = createWebListener(true);
		if (isSuccess == false) {
			printf("ERROR : Create web listener thread failed !\n");
		}*/
	pthread_join(webListernerThreadId, NULL);
	return 0;
}