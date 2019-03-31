#include "KeepAliveThread.h"

pthread_t keepAliveThreadId;
bool isOverFlag;

/*
***********************************************************
*
*	函数名	: keepAliveThread
*	功能	: 封装Keep-Alive状态下web响应的操作
*	参数	:
				【in】client_socket   : 用于传输的客户端socket号
*	返回值	: 【ret】isSuccess : 是否成功
*
***********************************************************
*/
bool keepAliveThread(int client_socket)
{
	printf("TIP : Thread keep alive runing ... \n");
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	/*异步取消， 线程接到取消信号后，立即退出*/
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	bool isSuccess = doKeepAlive(client_socket);
	printf("TIP : Keep-Alive over !\n");
	isOverFlag = false;
	return isSuccess;
}

/*
***********************************************************
*
*	函数名	: createKeepAliveThread
*	功能	: 开始创建Keep-Alive的线程
*	参数	:
				【in】client_socket   : 用于传输的客户端socket号
*	返回值	: 【ret】isSuccess : 是否成功
*
***********************************************************
*/
bool createKeepAliveThread(int client_socket)
{
	isOverFlag = true;
	int isSuccess = pthread_create(&keepAliveThreadId, NULL, (void*)keepAliveThread, &client_socket);
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
*	函数名	: cancelKeepAliveThread
*	功能	: 关闭web监听的线程
*	参数	: 无
				【in】client_socket   : 用于传输的客户端socket号
*	返回值	: 无
*
***********************************************************
*/
void cancelKeepAliveThread(int client_socket)
{
	void *retCode = NULL;
	close(client_socket);
	pthread_cancel(keepAliveThreadId);
	pthread_join(keepAliveThreadId, &retCode);
	isOverFlag = false;
	printf("TIP : Thread keep alive exit .\n");
}

/*
***********************************************************
*
*	函数名	: checkIsOver
*	功能	: 检查当前线程是否已经结束
*	参数	: 无
*	返回值	: 【ret】isSuccess : 是否成功
*
***********************************************************
*/
bool checkIsOver(void)
{
	return isOverFlag;
}