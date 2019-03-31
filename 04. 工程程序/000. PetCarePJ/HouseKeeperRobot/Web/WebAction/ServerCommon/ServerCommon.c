#include "ServerCommon.h"

int addressPort = 80;
char webAddressIP[BUFFERSIZE] = { 0 };
char webAddressPath[BUFFERSIZE] = { 0 };
char webAddressPara[BUFFERSIZE] = { 0 };
char webAddressPort[BUFFERSIZE] = { 0 };
char requestWebAddress[BUFFERSIZE] = { 0 };

char* contentTypeListReal[CONTENT_TYPESIZE] = {
	"BMP",
	"HTML",
	"JPEG",
	"JPG",
	"CSS",
	"JS",
	"MP3",
	"PNG",
	"GIF",
	"XML",
};

char* contentTypeListDefine[CONTENT_TYPESIZE] = {
	CONTENT_TYPE_BMP,
	CONTENT_TYPE_HTML,
	CONTENT_TYPE_JPEG,
	CONTENT_TYPE_JPG,
	CONTENT_TYPE_CSS,
	CONTENT_TYPE_JS,
	CONTENT_TYPE_MP3,
	CONTENT_TYPE_PNG,
	CONTENT_TYPE_GIF,
	CONTENT_TYPE_XML,
};

/*
***********************************************************
*
*	函数名	: initWebAddress
*	功能	: 初始化网址的各个模块
*	参数	: 无
*	返回值	: 无
*
***********************************************************
*/
void initWebAddress(void)
{
	// 解析出访问地址的各个部分
	bool isSuccess = false;
	isSuccess = ResolveIP(requestWebAddress, webAddressIP);
	if (isSuccess == false) {
		strcpy(webAddressIP, "192.168.1.1");
	}
	isSuccess = ResolvePort(requestWebAddress, webAddressPort);
	if (isSuccess == false) {
		strcpy(webAddressPort, "80");
		addressPort = atoi(webAddressPort);
	}
	isSuccess = ResolvePath(requestWebAddress, webAddressPath);
	if (isSuccess == false) {
		strcpy(webAddressPath, "/index.html");
	}
	isSuccess = ResolveParameter(requestWebAddress, webAddressPara);
	if (isSuccess == false) {
		strcpy(webAddressPara, "addid=remoteDo");
	}
}

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
bool ResolveProtocol(char* address, char* protocol)
{
	int i = 0;
	char *position = NULL;
	if ((position = strstr(address, "://")) != NULL) {
		for (i = 0; (address + i) != position; ++i) {
			protocol[i] = *(address + i);
		}
		protocol[i] = 0;
		return true;
	}
	else {
		return false;
	}
}

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
bool ResolveIP(char* address, char* ip)
{
	int i = 0;
	char *position = NULL;
	position = strstr(address, "://") + 3;
	if (position == NULL) {
		position = address;
	}

	for (i = 0; *(position + i) != '/' && *(position + i) != ':'; ++i) {
		ip[i] = *(position + i);
	}

	ip[i] = 0;

	return true;
}

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
bool ResolvePort(char* address, char* port)
{
	int i = 0;
	char *position = NULL;
	position = strstr(address, "://") + 3;
	position = strstr(position, ":");
	if (position == NULL) {
		port[0] = 0;
		return false;
	}
	position++;
	for (i = 0; *(position + i) != '/'; ++i) {
		port[i] = *(position + i);
	}
	port[i] = 0;
	return true;
}

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
bool ResolvePath(char* address, char* path)
{
	int i = 0;
	char *position = NULL;
	position = strstr(address, "://") + 3;
	position = strstr(position, "/");
	for (i = 0; *(position + i) != '?' && *(position + i); ++i) {
		path[i] = *(position + i);
	}
	path[i] = 0;
	return true;
}

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
bool ResolveParameter(char* address, char* parameter)
{
	char *position = NULL;
	position = strstr(address, "?");
	if (position == NULL) {
		parameter[0] = 0;
		return false;
	}
	position++;
	strcpy(parameter, position);
	return true;
}
