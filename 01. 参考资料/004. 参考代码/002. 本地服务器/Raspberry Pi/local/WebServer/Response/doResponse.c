#include "ResponseCommon.h"
#include "../ServerCommon/ServerCommon.h"

int serverSocketId = 0;
int webAddressDecodeTable[2][DECODETABLE] = {
	{ 0x20, 0x22, 0x23, 0x25, 0x26, 0x28, 0x29, 0x2B, 0x2c, 0x2F, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x5C, 0x7C },
	{ ' ', '"', '#', '%', '&', '(', ')', '+', ',', '/', ':', ';', '<', '=', '>', '?', '@', '\\', '|'}
};

/*
***********************************************************
*
*	函数名	: initSocketLink
*	功能	: 解析对象请求的报文
*	参数	:
				【out】server_socket   : 创建的socket_server id
*	返回值	: 【ret】 : 返回是否成功
*
***********************************************************
*/
bool initSocketLink(int *server_socket)
{
	struct sockaddr_in server_addr;
	// 创建服务器socket
	*server_socket = socket(AF_INET, SOCK_STREAM, 0);

	// 创建服务器ip结构
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(addressPort);

	// 绑定服务器socket与ip结构
	bool isSuccess = bind(*server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (isSuccess == false) {
		printf("ERROR : Server bind ip socket failure ! \n");
		return false;
	}

	// 开始监听socket
	if (listen(*server_socket, SIGNALVALUE) != 0) {
		printf("ERROR : Listen socket failure ! \n");
		return false;
	}

	return true;
}

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
bool checkMessageUseful(char *message)
{
	if (strstr(message, REQUEST_GET) == NULL && strstr(message, REQUEST_GET) == NULL) {
		return false;
	}

	if (strstr(message, "HTTP") == NULL) {
		return false;
	}

	return true;
}

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
void resolveMessage(char* message, char* contentType, char* requestPath)
{
	strcpy(contentType, CONTENT_TYPE_HTML);
	strcpy(requestPath, "/index.html");

	int i = 0, j = 0;
	char *requestPathBeginPosition = NULL, *requestPathEndPosition = NULL;
	// 获取客户端请求的文件路径
	if ((requestPathBeginPosition = strstr(message, "GET")) == NULL) {
		if ((requestPathBeginPosition = strstr(message, "POST")) != NULL) {
			requestPathBeginPosition = requestPathBeginPosition + 5;
		}
	}
	else {
		requestPathBeginPosition = requestPathBeginPosition + 4;
	}

	if ((requestPathEndPosition = strstr(message, "HTTP/")) != NULL) {
		requestPathEndPosition = requestPathEndPosition - 1;
	}

	// 如果取得的地址都成功
	if (requestPathBeginPosition && requestPathEndPosition) {
		for (i = 0; (requestPathBeginPosition + i) < requestPathEndPosition; ++i) {
			requestPath[i] = *(requestPathBeginPosition + i);
		}
		requestPath[i] = 0;

		if (strcmp(requestPath, "/") == 0) {
			strcpy(requestPath, "/index.html");
		}

		char fileContentType[BUFFERSIZE] = { 0 };
		char *tempBeginPosition = NULL, *tempEndPosition = NULL;
		strcpy(fileContentType, requestPath);
		tempBeginPosition = fileContentType;

		// 开始获取该文件类型
		if ((tempBeginPosition = strstr(tempBeginPosition, ".")) == NULL) {
			if (tempBeginPosition[strlen(tempBeginPosition)] == '/') {
				strcat(tempBeginPosition, "index.html");
			}
			else {
				strcpy(tempBeginPosition, "/index.html");
			}
		}
		else {
			tempBeginPosition = tempBeginPosition + 1;
		}

		if ((tempEndPosition = strstr(tempBeginPosition, "?")) == NULL) {
			tempEndPosition = tempBeginPosition + strlen(tempBeginPosition);
		}
		else {
			tempEndPosition = tempEndPosition - 1;
		}

		for (i = 0; (tempBeginPosition + i) <= tempEndPosition; ++i) {
			fileContentType[i] = *(tempBeginPosition + i);
		}
		fileContentType[i] = 0;

		for (i = 0; i < CONTENT_TYPESIZE; ++i) {
			for (j = 0; j < strlen(fileContentType); ++j) {
				if (fileContentType[j] >= 'a' && fileContentType[j] <= 'z') {
					fileContentType[j] = fileContentType[j] - 32;
				}
			}
			if (strcmp(fileContentType, contentTypeListReal[i]) == 0) {
				strcpy(contentType, contentTypeListDefine[i]);
				break;
			}
		}

		if (i == CONTENT_TYPESIZE) {
			printf("ERROR : Can't support file type %s ! \n", fileContentType);
		}
	}

	char tempBuffer[BUFFERSIZE] = { 0 };
	strcpy(tempBuffer, "../web/web");
	strcat(tempBuffer, requestPath);
	strcpy(requestPath, tempBuffer);

	printf("TIP : Resolve content type is [%s] . \n", contentType);
	printf("TIP : Resolve request path is [%s] . \n", requestPath);
}

/*
***********************************************************
*
*	函数名	: resolveParameter
*	功能	: 解析对象请求的报文
*	参数	:
				【in】message   : 做成的报文
				【out】parameterStack   : 用户报文的参数队列
*	返回值	: 无
*
***********************************************************
*/
void resolveParameter(char* message, ParameterStack *parameterStack)
{
	int i = 0;
	char *requestParameterBeginPosition = NULL, *requestParameterEndPosition = NULL;
	requestParameterEndPosition = strstr(message, "HTTP/");
	if (requestParameterEndPosition == NULL) {
		return;
	}
	else {
		requestParameterEndPosition = requestParameterEndPosition - 1;
	}

	requestParameterBeginPosition = strstr(message, "?");
	if (requestParameterBeginPosition == NULL ||
		requestParameterBeginPosition > requestParameterEndPosition) {
		return;
	}
	else {
		requestParameterBeginPosition = requestParameterBeginPosition + 1;
	}

	int tIndex = 0;
	char tempBuffer[BUFFERSIZE] = { 0 };
	for (i = 0; (requestParameterBeginPosition + i) != requestParameterEndPosition; ++i) {
		switch (*(requestParameterBeginPosition + i))
		{
		case '=':
			tempBuffer[tIndex] = 0;
			parameterStack->parameterKey[parameterStack->parameterLen] = malloc(sizeof(char) * strlen(tempBuffer));
			strcpy(parameterStack->parameterKey[parameterStack->parameterLen], tempBuffer);
			tIndex = 0;
			break;
		case '&':
			tempBuffer[tIndex] = 0;
			parameterStack->parameterValue[parameterStack->parameterLen] = malloc(sizeof(char) * strlen(tempBuffer));
			strcpy(parameterStack->parameterValue[parameterStack->parameterLen], tempBuffer);
			parameterStack->parameterLen++;
			tIndex = 0;
			break;
		default:
			tempBuffer[tIndex++] = *(requestParameterBeginPosition + i);
		}
	}

	tempBuffer[tIndex] = 0;
	parameterStack->parameterValue[parameterStack->parameterLen] = malloc(sizeof(char) * strlen(tempBuffer));
	strcpy(parameterStack->parameterValue[parameterStack->parameterLen], tempBuffer);
	parameterStack->parameterLen++;

	printf("TIP : Resolve parameter list : \n");
	printf("%-20s\t%-10s\n", "Key", "Value");
	for (i = 0; i < parameterStack->parameterLen; ++i) {
		printf("%-20s\t%-10s\n", parameterStack->parameterKey[i], parameterStack->parameterValue[i]);
	}
	printf("\n");
}

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
void makeMessageHead(char* contentType, bool isKeepLink, char* message)
{
	// 制作报文信息
	strcat(message, "HTTP/1.0 200 OK\r\n");
	strcat(message, "Server: DWBServer\r\n");
	if (isKeepLink == true) {
		strcat(message, "Connection: Keep-Alive\r\n");
	}
	sprintf(message, "%sContent-Type: %s;charset=utf-8\r\n\r\n", message, contentType);
}

/*
***********************************************************
*
*	函数名	: setWebAddressParameter
*	功能	: 设置全局的向MFC发送请求的网址
*	参数	:
				【in】parameterStack   : 用户报文的参数队列
*	返回值	: 无
*
***********************************************************
*/
bool setWebAddressParameter(ParameterStack *parameterStack)
{
	int i = 0;
	bool isSuccess = false;
	// 检查密码是否正确
	for (i = 0; i < parameterStack->parameterLen; ++i) {
		if (strcmp(parameterStack->parameterKey[i], "userName") == 0) {
			if (strcmp(parameterStack->parameterValue[i], "admin") == 0) {
				isSuccess = true;
				break;
			}
		}
	}

	if (isSuccess == false) {
		return false;
	}

	// 设置参数
	for (i = 0; i < parameterStack->parameterLen; ++i) {
		if (strcmp(parameterStack->parameterKey[i], "requestAddress") == 0) {
			strcpy(requestWebAddress, parameterStack->parameterValue[i]);
			printf("TIP : Set request web address is %s . \n", requestWebAddress);
			break;
		}
	}

	if (i == parameterStack->parameterLen) {
		return false;
	}


	return true;
}

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
void freeWebAddressParameter(ParameterStack *parameterStack)
{
	int i = 0;
	for (i = 0; i < parameterStack->parameterLen; ++i) {
		free(parameterStack->parameterKey[i]);
		free(parameterStack->parameterValue[i]);
	}
	parameterStack->parameterLen = 0;
}

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
void decodingWebAddress(char *webAddress)
{
	int i = 0, j = 0, wIndex = 0;
	for (i = 0; i < strlen(webAddress); ++i) {
		// 查找到转义字符的位置
		if (webAddress[i] == '%') {
			int decode = 0, tempNumber = 0;
			tempNumber = webAddress[i + 1] >= 'A' ? webAddress[i + 1] - 'A' + 10 : webAddress[i + 1] - '0';
			decode = decode + tempNumber * 16;
			tempNumber = webAddress[i + 2] >= 'A' ? webAddress[i + 2] - 'A' + 10 : webAddress[i + 2] - '0';
			decode = decode + tempNumber;
			for (j = 0; j < DECODETABLE; ++j) {
				if (webAddressDecodeTable[0][j] == decode) {
					webAddress[wIndex++] = webAddressDecodeTable[1][j];
					break;
				}
			}
			i += 2;
		}
		else {
			webAddress[wIndex++] = webAddress[i];
		}
	}
	webAddress[wIndex] = 0;
}

/*
***********************************************************
*
*	函数名	: handleWebAddressParameter
*	功能	: 清除参数内存
*	参数	:
				【in】parameterStack   : 用户报文的参数队列
				【out】messgae   : 处理后的消息
*	返回值	: 无
*
***********************************************************
*/
void handleWebAddressParameter(ParameterStack *parameterStack, char *message)
{
	bool isSuccess = setWebAddressParameter(parameterStack);
	freeWebAddressParameter(parameterStack);
	decodingWebAddress(requestWebAddress);
	if (isSuccess == true) {
		strcpy(message, "<html><head></head><body><h3>Set Parameter Success !</h3><br/>");
		sprintf(message, "%s<h3>You set MFC Request Address = [%s]</h3><br/>", message, requestWebAddress);
		sprintf(message, "%s<a href='index.html'>Back index</a></body></html>\r\n\r\n", message);

	}
	else {
		strcpy(message, "<html><head></head><body><h3>Set Parameter failed !</h3><br/>");
		sprintf(message, "%s<a href='index.html'>Back index</a></body></html>\r\n\r\n", message);
	}
}

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
bool responseFileData(char *requestPath, char *message, int client_socket, char *contentType)
{
	FILE *fp = fopen(requestPath, "rb");
	if (fp == NULL) {
		printf("ERROR : [%s] Can't find this file ! \n", requestPath);
		return false;
	}

	printf("TIP : Send file data ... \n");
	do {
		memset(message, 0, BUFFERSIZE);
		int readNumber = fread(message, sizeof(char), BUFFERSIZE, fp);
		write(client_socket, message, readNumber);
		if (strncasecmp(contentType, "text", strlen("text")) == 0 ||
			strncasecmp(contentType, "application/x-javascript", strlen("application/x-javascript")) == 0) {
			printf("%s", message);
		}
	} while (!feof(fp));
	fclose(fp);
	printf("\n");
	printf("TIP : Send file data over ! \n");

	return true;
}

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
bool doResponse(bool keepListern)
{
	int isSuccess = 0;
	int server_socket = 0;
	bool isKeepAlive = false;
	char message[BUFFERSIZE] = { 0 };
	// 初始化socket的一些数据
	isSuccess = initSocketLink(&server_socket);
	if (isSuccess == false) {
		return false;
	}

	// 设置全局变量，方便后面关闭线程时保守
	serverSocketId = server_socket;
	do {
		printf("============================================= \n");
		printf("TIP : Wating link ... \n");
		int client_socket = accept(server_socket, NULL, NULL);
		printf("TIP : You have a new Link ! \n");

		// 读取对象发过来的报文信息
		printf("TIP : Reading ... \n");
		memset(message, 0, BUFFERSIZE);
		int size = read(client_socket, message, BUFFERSIZE);
		printf("TIP : Receve the request : \n%s\n", message);

		if (checkMessageUseful(message) == false) {
			printf("ERROR : This not a usefule message ! \n");
			continue;
		}

		// 如果当前为Keep-Alive模式
		if (strncasecmp(strstr(message, "Connection"), "Connection: Keep-Alive",
			strlen("Connection: Keep-Alive")) == 0) {
			isKeepAlive = true;
		}
		else {
			isKeepAlive = false;
		}

		char contentType[BUFFERSIZE], requestPath[BUFFERSIZE];
		resolveMessage(message, contentType, requestPath);

		// 解析请求中的参数队列
		ParameterStack parameterStack;
		parameterStack.parameterLen = 0;
		resolveParameter(message, &parameterStack);

		// 制作将要返回的电文头信息
		memset(message, 0, BUFFERSIZE);
		if (parameterStack.parameterLen != 0) {
			makeMessageHead(CONTENT_TYPE_HTML, false, message);
		}
		else {
			makeMessageHead(contentType, false, message);
		}
		write(client_socket, message, strlen(message));
		printf("%s\n", message);

		if (parameterStack.parameterLen != 0) {
			handleWebAddressParameter(&parameterStack, message);
			write(client_socket, message, strlen(message));
			printf("%s\n", message);
			close(client_socket);
			continue;
		}

		// 发送请求的path地址的数据
		isSuccess = responseFileData(requestPath, message, client_socket, contentType);
		if (isSuccess == false) {
			continue;
		}

		
		if (isKeepAlive == true) {
			printf("TIP : This is a Keep-Alive Link ! \n");
			struct timeval timeBegin, timeEnd;
			createKeepAliveThread(client_socket);
			gettimeofday(&timeBegin, 0);
			while (true) {
				if (checkIsOver() == false) {
					printf("TIP : Keep Alive Thread is Over ! \n");
					break;
				}

				// 超时检测
				gettimeofday(&timeEnd, 0);
				if ((timeEnd.tv_sec - timeBegin.tv_sec) > OVERTIME) {
					printf("TIP : Over time %ds . \n", (timeEnd.tv_sec - timeBegin.tv_sec));
					printf("TIP : Over time ! Finish link ! \n");
					cancelKeepAliveThread(client_socket);
					break;
				}
			}
			close(client_socket);
			continue;
		}

		close(client_socket);
	} while (keepListern);

	close(server_socket);

	return true;
}
