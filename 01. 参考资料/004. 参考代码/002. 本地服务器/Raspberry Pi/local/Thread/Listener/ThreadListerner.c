#include "ThreadListerner.h"


pthread_t webListernerThreadId;

/*
***********************************************************
*
*	函数名	: webListernerThread
*	功能	: 封装web响应的操作
*	参数	:
				【in】keepListern   : 用户是否要要在响应一次请求之后持续监听
*	返回值	: 【ret】isSuccess : 是否成功
*
***********************************************************
*/
bool webListernerThread(bool keepListen)
{
	printf("TIP : Web listern thread is running ... \n");
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	/*异步取消， 线程接到取消信号后，立即退出*/
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	bool isSuccess = doResponse(keepListen);

	return isSuccess;
}

/*
***********************************************************
*
*	函数名	: createWebListener
*	功能	: 开始创建web服务器监听的线程
*	参数	:
				【in】keepListern   : 用户是否要要在响应一次请求之后持续监听
*	返回值	: 【ret】isSuccess : 是否成功
*
***********************************************************
*/
bool createWebListener(bool keepListen)
{
	int isSuccess = pthread_create(&webListernerThreadId, NULL, (void*)webListernerThread, &keepListen);
	if (isSuccess != 0) {
		return false;
	}
	else {
		return true;
	}
}

/*
***********************************************************
*
*	函数名	: cancelWebListerner
*	功能	: 关闭web监听的线程
*	参数	: 无
*	返回值	: 无
*
***********************************************************
*/
void cancelWebListerner(void)
{
	void *retCode = NULL;
	close(serverSocketId);
	pthread_cancel(webListernerThreadId);
	pthread_join(webListernerThreadId, &retCode);
	printf("TIP : Thread web listern exit .\n");
}