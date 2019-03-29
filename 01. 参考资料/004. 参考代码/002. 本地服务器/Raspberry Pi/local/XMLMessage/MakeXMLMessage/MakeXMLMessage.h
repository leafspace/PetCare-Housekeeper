#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "../ASRQueue/ASRQueue.h"
#include "../../AllCommon/Common.h"
#include "../../UserKeyword/CommonKeyword.h"
#include "../../UserKeyword/CommonKeywordCode.h"

/*
***********************************************************
*
*	函数名	: makeXMLMessage
*	功能	: 将字符串转为识别的关键词码队列
*	参数	:
				【in】queue   : 语音识别关键词码队列
*	返回值	: 【ret】isSuccess : 是否成功
*
***********************************************************
*/
bool makeXMLMessage(ASRQueue *queue);