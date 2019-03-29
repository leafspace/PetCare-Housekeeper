运行该部分程序：[注]：输入命令时同一条命令中不能有回车
#!/bin/sh
gcc ComCommon/ComCommon.c UserKeyword/CommonKeyword.c WebServer/Request/doRequest.c WebServer/Response/doResponse.c WebServer/Response/doKeepAlive.c 
WebServer/ServerCommon/ServerCommon.c MainCommon/MainCommon.c Thread/Listener/ThreadListerner.c Thread/KeepAlive/KeepAliveThread.c XMLMessage/ASRQueue/ASRQueue.c 
XMLMessage/MakeXMLMessage/MakeXMLMessage.c UnitTest/TestMain.c -lpthread -o VoicePrinter
./VoicePrinter

运行该测试程序：
#!/bin/sh
gcc ComCommon/ComCommon.c UserKeyword/CommonKeyword.c WebServer/Request/doRequest.c WebServer/Response/doResponse.c WebServer/Response/doKeepAlive.c 
WebServer/ServerCommon/ServerCommon.c MainCommon/MainCommon.c Thread/Listener/ThreadListerner.c Thread/KeepAlive/KeepAliveThread.c XMLMessage/ASRQueue/ASRQueue.c 
XMLMessage/MakeXMLMessage/MakeXMLMessage.c UnitTest/TestMain.c -lpthread -o UnitTest
./UnitTest.exe




Q：因为非root用户无法获得读取串口的数据
    临时：sudo chmod 777 /dev/ttyUSB0
    永久：sudo usermod -aG dialout 【当前用户名】



KnowHow:
1. 如何获取本地->target或target->本地的报文信息？
    A：使用Fiddler软件，打开软件后在右侧的工具栏依次选择Inspectors->Raw则可看到报文信息。
2. 查找Nginx服务器的进程号，并关闭Nginx进程
    [注]: 此动作需要超级管理员权限
    sudo ps -ef|grep nginx
    sudo kill -quit [id]


来自浏览器请求树莓派服务器的请求
GET / HTTP/1.1
Host: localhost
Connection: keep-alive
Upgrade-Insecure-Requests: 1
User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/67.0.3396.79 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng;q=0.8
Accept-Encoding: gzip, deflate, br
Accept-Language: zh-CN,zh;q=0.9


通过浏览器URL启动MFC
http://[MFC IP]/openinterface/kickapp?appid=[idName]

通过浏览器设置树莓派服务器的请求参数
http://[PI IP]/index.html?userName=admin&requestAddress=http%3A%2F%2F[MFC IP]%2Fopeninterface%2Fkickapp%3Fappid%3D[idName]

[注]：http%3A%2F%2F[MFC IP]%2Fopeninterface%2Fkickapp%3Fappid%3D[idName] 为URL转义后的结果


UnitTest.exe&VoicePrinter.exe可以在MSYS64中直接运行

