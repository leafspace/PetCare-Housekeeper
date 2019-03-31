/*
***********************************************************
*
*	函数名	: sendRequest
*	功能	: 向定义的目标主机发送get请求
*	参数	:
				【in】requestWay   : 用户使用的请求方式：例如GET或POST
				【in】contentType   : 用户要请求的类型：例如text/html
				【in】keepRecive   : 用户是否要在发送结束后继续监听返回的数据
*	返回值	: 【ret】isSuccess : 是否成功
*
***********************************************************
*/
bool sendRequest(char *requestWay, char *contentType, bool keepRecive)
{
	bool isSuccess = true;
	cancelWebListerner();
	initWebAddress();
	isSuccess = doRequest(requestWay, contentType, keepRecive);
	if (isSuccess == false) {
		printf("ERROR : Send <GET> request failed !\n");
		return false;
	}

	isSuccess = createWebListener(true);
	if (isSuccess == false) {
		printf("ERROR : Create web listener thread failed !\n");
		return false;
	}

	return true;
}