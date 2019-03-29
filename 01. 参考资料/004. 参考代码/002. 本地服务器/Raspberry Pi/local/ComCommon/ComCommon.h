#pragma once

#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "../AllCommon/Common.h"

extern const int32_t name_arr[];
extern const int32_t speed_arr[];

/*
***********************************************************
*
*	函数名	: set_speed
*	功能	: 设置波特率
*	参数	:
*	返回值	:
*
***********************************************************
*/
bool set_speed(int, int);

/*
***********************************************************
*
*	函数名	: set_parity
*	功能	: 设置设备参数
*	参数	:
*	返回值	:
*
***********************************************************
*/
bool set_parity(int, int, int, int);

/*
***********************************************************
*
*	函数名	: findUsefulDriveList
*	功能	: 查找范围内可用设备
*	参数	:
				【in】driveSize   : 设备号范围
				【out】driveID    : 设备号保存处
*	返回值	: 【ret】usefulDriveNum    :可用设备数量
*
***********************************************************
*/
int findUsefulDriveList(int driveSize, int *driveID);

/*
***********************************************************
*
*	函数名	: closeUsefulDriveList
*	功能	: 关闭所有打开的设备
*	参数	:
				【in】driveSize  : 设备号范围
				【in】driveID    : 设备号保存处
*	返回值	: 无
*
***********************************************************
*/
void closeUsefulDriveList(int driveSize, int *driveID);