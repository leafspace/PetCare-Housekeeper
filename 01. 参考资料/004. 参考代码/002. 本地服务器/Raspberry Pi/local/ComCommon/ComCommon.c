#include "ComCommon.h"

const int32_t name_arr[] = { B0, B50, B75, B110, B134, B150, B200, B300, B600, B1200, B1800, B2400, B9600, B38400, B57600, B115200 };
const int32_t speed_arr[] = { 0, 50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400, 9600, 38400, 57600, 115200 };

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
bool set_speed(int comID, int speed)
{
	int i = 0, status = 0;
	struct termios optTermios;
	tcgetattr(comID, &optTermios);
	int minSize = sizeof(speed_arr) > sizeof(name_arr) ? sizeof(name_arr) : sizeof(speed_arr);
	for (i = 0, status = 0; i < minSize / sizeof(int); ++i) {
		if (speed == speed_arr[i]) {
			tcflush(comID, TCIOFLUSH);
			cfsetispeed(&optTermios, name_arr[i]);
			cfsetospeed(&optTermios, name_arr[i]);
			status = tcsetattr(comID, TCSANOW, &optTermios);
			if (status != 0) {
				return false;
			}
			break;
		}

		// 清空未接收的字符
		tcflush(comID, TCIOFLUSH);
	}
	return true;
}

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
bool set_parity(int comID, int dataBits, int stopBits, int parity)
{
	struct termios optTermios;
	if (tcgetattr(comID, &optTermios) != 0) {
		return false;
	}
	optTermios.c_cflag &= ~CSIZE;

	switch (dataBits)
	{
	case 7:
		optTermios.c_cflag |= CS7;
		break;
	case 8:
		optTermios.c_cflag |= CS8;
		break;
	default:
		return false;
	}

	switch (parity)
	{
	case 'n': case 'N':
		optTermios.c_cflag &= ~PARENB;
		optTermios.c_iflag &= ~INPCK;
		break;
	case 'o': case 'O':
		optTermios.c_cflag |= (PARODD | PARENB);
		optTermios.c_iflag |= INPCK;
		break;
	case 'e': case 'E':
		optTermios.c_cflag |= PARENB;
		optTermios.c_cflag &= ~PARODD;
		optTermios.c_iflag |= INPCK;
		break;
	case 'S': case 's':
		optTermios.c_cflag &= ~PARENB;
		optTermios.c_cflag &= ~CSTOPB; break;
	default:
		return false;
	}

	switch (stopBits)
	{
	case 1:
		optTermios.c_cflag &= ~CSTOPB;
		break;
	case 2:
		optTermios.c_cflag |= CSTOPB;
		break;
	default:
		return false;
	}

	if (parity != 'n') {
		optTermios.c_iflag |= INPCK;
	}
	tcflush(comID, TCIFLUSH);
	optTermios.c_cc[VTIME] = 150;
	optTermios.c_cc[VMIN] = 0;
	if (tcsetattr(comID, TCSANOW, &optTermios) != 0) {
		return false;
	}
	return true;
}

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
int findUsefulDriveList(int driveSize, int *driveID)
{
	int i = 0;
	int usefulDriveNum = 0;
	char driveName[15] = { 0 };
	for (i = 0; i < driveSize; ++i) {
		sprintf(driveName, "/dev/ttyUSB%d", i);
		int comFId = open(driveName, O_RDWR);
		driveID[i] = comFId;
		if (driveID[i] > 0) {
			usefulDriveNum++;
		}
	}
	return usefulDriveNum;
}

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
void closeUsefulDriveList(int driveSize, int *driveID)
{
	int i = 0;
	for (i = 0; i < driveSize; ++i) {
		close(driveID[i]);
	}
}