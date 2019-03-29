#pragma once

#include <pthread.h>
#include "../../AllCommon/Common.h"
#include "../../WebServer/Response/ResponseCommon.h"
#include "../../WebServer/ServerCommon/ServerCommon.h"

extern pthread_t webListernerThreadId;

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
bool webListernerThread(bool keepListen);

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
bool createWebListener(bool keepListen);

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
void cancelWebListerner(void);