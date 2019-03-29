#pragma once
#define SIGNALVALUE 1                                                       // socket 同时接受的连接数量
#define STACKSIZE 20                                                        // 队列的数据数量
#define DECODETABLE 19                                                      // 解码列表的长度
#define OVERTIME 3                                                          // 超时的时常

#include "../ServerCommon/ServerCommon.h"

typedef struct ParameterStack
{
	char *parameterKey[STACKSIZE];
	char *parameterValue[STACKSIZE];
	int parameterLen;
} ParameterStack;

extern int serverSocketId;
extern int webAddressDecodeTable[2][DECODETABLE];

/*
***********************************************************
*
*	函数名	: initSocketLink
*	功能	: socket接受请求前的一些初始化
*	参数	:
				【out】server_socket   : 创建的socket_server id
*	返回值	: 【ret】 isSuccess: 返回是否成功
*
***********************************************************
*/
bool initSocketLink(int *server_socket);

/*
***********************************************************
*
*	函数名	: checkMessageUseful
*	功能	: 检查是否是一个正常的报文
*	参数	:
				【in】message   : 客户端请求的报文
*	返回值	: 【ret】 isSuccess: 返回是否成功
*
***********************************************************
*/
bool checkMessageUseful(char *messgae);

/*
***********************************************************
*
*	函数名	: resolveMessage
*	功能	: 解析对象请求的报文
*	参数	:
				【in】message   : 做成的报文
				【out】contentType   : 用户要请求的类型：例如text/html
				【out】requestPath   : 用户要请求的文件路径
*	返回值	: 无
*
***********************************************************
*/
void resolveMessage(char* message, char* contentType, char* requetPath);

/*
***********************************************************
*
*	函数名	: resolveParameter
*	功能	: 解析对象请求的报文做成参数队列
*	参数	:
				【in】message   : 做成的报文
				【out】parameterStack   : 用户报文的参数队列
*	返回值	: 无
*
***********************************************************
*/
void resolveParameter(char* message, ParameterStack *parameterStack);

/*
***********************************************************
*
*	函数名	: makeMessageHead
*	功能	: 制作要发送给mfc的报文
*	参数	:
				【in】contentType   : 用户要请求的类型：例如text/html
				【out】message   : 做成的报文
*	返回值	: 无
*
***********************************************************
*/
void makeMessageHead(char* contentType, bool isKeepLink, char* message);

/*
***********************************************************
*
*	函数名	: setWebAddressParameter
*	功能	: 设置全局的向MFC发送请求的网址
*	参数	:
				【in】parameterStack   : 用户报文的参数队列
*	返回值	: 【ret】 isSuccess: 返回是否成功
*
***********************************************************
*/
bool setWebAddressParameter(ParameterStack *parameterStack);

/*
***********************************************************
*
*	函数名	: freeWebAddressParameter
*	功能	: 清除参数内存
*	参数	:
				【in】parameterStack   : 用户报文的参数队列
*	返回值	: 无
*
***********************************************************
*/
void freeWebAddressParameter(ParameterStack *parameterStack);

/*
***********************************************************
*
*	函数名	: decodingWebAddress
*	功能	: 为特殊字符转义过的网址解码
*	参数	:
				【in|out】webAddress   : web网址
*	返回值	: 无
*
***********************************************************
*/
void decodingWebAddress(char *webAddress);

/*
***********************************************************
*
*	函数名	: handleWebAddressParameter
*	功能	: 处理解析出来的参数
*	参数	:
				【in】parameterStack   : 用户报文的参数队列
				【out】pmessgae   : 处理后的消息
*	返回值	: 无
*
***********************************************************
*/
void handleWebAddressParameter(ParameterStack *parameterStack, char *message);

/*
***********************************************************
*
*	函数名	: responseFileData
*	功能	: 响应发送的请求文件
*	参数	:
				【in】requestPath   : 用户报文的参数队列
				【in】messgae   : 处理后的消息
				【in】client_socket   : 客户端的socket
				【in】contentType   : 处理后的消息
*	返回值	: 【ret】 isSuccess: 返回是否成功
*
***********************************************************
*/
bool responseFileData(char *requestPath, char *message, int client_socket, char *contentType);