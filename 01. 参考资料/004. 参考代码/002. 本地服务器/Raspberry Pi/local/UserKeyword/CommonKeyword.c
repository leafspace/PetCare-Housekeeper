#include "CommonKeyword.h"

/*添加关键词，用户修改*/
uint8 sRecog[DATE_ROW][DATE_CAL] = {
	"huan xing",
	"kai fa ban yan zheng",
	"dai ma ce shi",
	"kao bei",
	"sao miao",
	"yi",
	"er",
	"san",
	"si",
	"wu",
	"liu",
	"qi",
	"ba",
	"jiu",
	"shi",
	"bai",
	"qian",
	"wan",
	"zhang",
	"ye",
	"ci",
	"jieshu"
};

/*添加识别码，用户修改*/
uint8 pCode[DATE_ROW] = {
	CODE_CMD,
	CODE_KFBYZ,
	CODE_DMCS,
	CODE_COPY,
	CODE_SCAN,
	CODE_01,
	CODE_02,
	CODE_03,
	CODE_04,
	CODE_05,
	CODE_06,
	CODE_07,
	CODE_08,
	CODE_09,
	CODE_10,
	CODE_BAI,
	CODE_QIAN,
	CODE_WAN,
	CODE_ZHANG,
	CODE_YE,
	CODE_CI,
	CODE_jieshu,
};