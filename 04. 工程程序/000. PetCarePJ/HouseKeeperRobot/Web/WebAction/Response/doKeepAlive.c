#include "ResponseCommon.h"
#include "../ServerCommon/ServerCommon.h"
#include <sys/types.h>
#include <asm/byteorder.h>
#include <sys/config.h>
//#include <sys/skbuff.h>
//#include <sys/ip.h>
//#include <net/sock.h>

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
bool doKeepAlive(int client_socket)
{
	int isSuccess = 0;
	char message[BUFFERSIZE] = { 0 };

	do
	{
		printf("TIP : Reading ... \n");
		memset(message, 0, BUFFERSIZE);
		int length = read(client_socket, message, BUFFERSIZE);
        if (length <= 0) {
            printf("TIP : Client socket breaked ! \n");
            break;
        }
		printf("TIP : Receve the request : \n%s\n", message);
        if (checkMessageUseful(message) == false) {
			printf("ERROR : This not a usefule message ! \n");
			continue;
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
			makeMessageHead(CONTENT_TYPE_HTML, true, message);
		}
		else {
			makeMessageHead(contentType, true, message);
		}
		write(client_socket, message, strlen(message));
		printf("%s\n", message);

		if (parameterStack.parameterLen != 0) {
			handleWebAddressParameter(&parameterStack, message);
			write(client_socket, message, strlen(message));
			printf("%s\n", message);
			continue;
		}

		// 发送请求的path地址的数据
		isSuccess = responseFileData(requestPath, message, client_socket, contentType);
		if (isSuccess == false) {
			continue;
		}

	} while (true);

	return true;
}