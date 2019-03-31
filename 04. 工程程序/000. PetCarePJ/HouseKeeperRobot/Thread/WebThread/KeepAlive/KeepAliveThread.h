#pragma once

#include <pthread.h>
#include "../../AllCommon/Common.h"
#include "../../WebServer/Response/ResponseCommon.h"
#include "../../WebServer/ServerCommon/ServerCommon.h"

extern pthread_t keepAliveThreadId;

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
bool keepAliveThread(int client_socket);

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
bool createKeepAliveThread(int client_socket);

/*
***********************************************************
*
*	函数名	: cancelKeepAliveThread
*	功能	: 关闭web监听的线程
*	参数	: 
				【in】client_socket   : 用于传输的客户端socket号
*	返回值	: 无
*
***********************************************************
*/
void cancelKeepAliveThread(int client_socket);

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
bool checkIsOver(void);