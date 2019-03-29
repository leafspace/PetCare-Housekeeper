#ifndef __COMMONKEYWORD_H
#define __COMMONKEYWORD_H

#include "CommonKeywordCode.h"

#define DATE_ROW 22      /*关键词个数*/
#define DATE_CAL 20		 /*关键词最大长度*/

#define uint8 unsigned char

extern uint8 sRecog[DATE_ROW][DATE_CAL];
extern uint8 pCode[DATE_ROW];

#endif