#pragma once

#define REQUEST_GET "GET"
#define REQUEST_POST "POST"
#define REQUEST_OPTIONS "OPTIONS"
#define REQUEST_HEAD "HEAD"
#define REQUEST_PUT "PUT"
#define REQUEST_DELETE "DELETE"
#define REQUEST_TRACE "TRACE"
#define REQUEST_CONNECT "CONNECT"

#define CONTENT_TYPE_BMP "application/x-bmp"
#define CONTENT_TYPE_HTML "text/html"
#define CONTENT_TYPE_JPEG "image/jpeg"
#define CONTENT_TYPE_JPG "image/jpg"
#define CONTENT_TYPE_CSS "text/css"
#define CONTENT_TYPE_JS "application/x-javascript"
#define CONTENT_TYPE_MP3 "audio/mp3"
#define CONTENT_TYPE_PNG "image/png"
#define CONTENT_TYPE_GIF "image/gif"
#define CONTENT_TYPE_XML "text/xml"

#define CONTENT_TYPESIZE 10
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <strings.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "../../AllCommon/Common.h"
#include "../../Thread/KeepAlive/KeepAliveThread.h"

extern char* contentTypeListReal[];
extern char* contentTypeListDefine[];

extern int addressPort;
extern char webAddressIP[BUFFERSIZE];
extern char webAddressPath[BUFFERSIZE];
extern char webAddressPara[BUFFERSIZE];
extern char webAddressPort[BUFFERSIZE];
extern char requestWebAddress[BUFFERSIZE];

/*
***********************************************************
*
*	函数名	: doRequest
*	功能	: 向定义的目标主机发送get请求
*	参数	:
				【in】requestWay   : 用户使用的请求方式：例如GET或POST
				【in】contentType   : 用户要请求的类型：例如text/html
				【in】keepRecive   : 用户是否要在发送结束后继续监听返回的数据
*	返回值	: 【ret】isSuccess : 是否成功
*
***********************************************************
*/
bool doRequest(char* requestWay, char* contentType, bool keepRecive);

/*
***********************************************************
*
*	函数名	: doResponse
*	功能	: 响应浏览器发送过来的请求
*	参数	:
				【in】keepListern   : 用户是否要要在响应一次请求之后持续监听
*	返回值	: 【ret】isSuccess : 是否成功
*
***********************************************************
*/
bool doResponse(bool keepListern);

/*
***********************************************************
*
*	函数名	: doKeepAlive
*	功能	: 启动Keep-Alive模式的监听
*	参数	:
				【in】client_socket   : 用于传输的客户端socket号
*	返回值	: 【ret】isSuccess : 是否成功
*
***********************************************************
*/
bool doKeepAlive(int client_socket);

/*
***********************************************************
*
*	函数名	: doRequest
*	功能	: 向定义的目标主机发送get请求
*	参数	: 无
*	返回值	: 无
*
***********************************************************
*/
void initWebAddress(void);

/*
***********************************************************
*
*	函数名	: ResolveProtocol
*	功能	: 解析ip网址使用的协议
*	参数	:
				【in】address   : 用户设置的ip地址
				【out】protocol : 目标使用的协议
*	返回值	: 【ret】isSuccess : 是否成功
*
***********************************************************
*/
bool ResolveProtocol(char* address, char* protocol);
/*
***********************************************************
*
*	函数名	: ResolveIP
*	功能	: 解析ip网址使用的ip
*	参数	:
				【in】address   : 用户设置的ip地址
				【out】ip       : 目标使用的ip
*	返回值	: 【ret】isSuccess : 是否成功
*
***********************************************************
*/
bool ResolveIP(char* address, char* ip);
/*
***********************************************************
*
*	函数名	: ResolvePort
*	功能	: 解析ip网址使用的端口号
*	参数	:
				【in】address   : 用户设置的ip地址
				【out】port     : 目标访问机器的端口号
*	返回值	: 【ret】isSuccess : 是否成功
*
***********************************************************
*/
bool ResolvePort(char* address, char* port);
/*
***********************************************************
*
*	函数名	: ResolvePath
*	功能	: 解析ip网址所访问的路径
*	参数	:
				【in】address   : 用户设置的ip地址
				【out】path     : 目标所要访问的路径
*	返回值	: 【ret】isSuccess : 是否成功
*
***********************************************************
*/
bool ResolvePath(char* address, char* path);
/*
***********************************************************
*
*	函数名	: ResolveParameter
*	功能	: 解析ip网址附加的参数
*	参数	:
				【in】address   : 用户设置的ip地址
				【out】parameter: 目标所附带的参数
*	返回值	: 【ret】isSuccess : 是否成功
*
***********************************************************
*/
bool ResolveParameter(char* address, char* parameter);
