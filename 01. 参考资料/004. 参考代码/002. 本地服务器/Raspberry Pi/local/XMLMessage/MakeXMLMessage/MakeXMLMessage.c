#include "MakeXMLMessage.h"

/*
***********************************************************
*
*	函数名	: makeXMLMessage
*	功能	: 将字符串转为识别的关键词码队列
*	参数	:
				【in】queue   : 语音识别关键词码队列
*	返回值	: 【ret】isSuccess : 是否成功
*
***********************************************************
*/
bool makeXMLMessage(ASRQueue *queue)
{
	int keyNum = 0;
	printf("TIP : ASR reconation value list : \n");
	while (queue->asrQueueLen > 0) {
		popASRQueue(queue, &keyNum);
		switch (keyNum)
		{
		case CODE_COPY: system("cp ../web/web/xml/iocopy/IoCopy.xml ../web/web/xml/UserMake/define.xml"); break;
		case CODE_SCAN: system("cp ../web/web/xml/ioscan/scan2media/scan.xml ../web/web/xml/UserMake/define.xml"); break;
		default: break;
		}
		printf("%d\t", keyNum);
	}
	printf("\n");
	printf("TIP : Make message file OK ! \n");
	return true;
}