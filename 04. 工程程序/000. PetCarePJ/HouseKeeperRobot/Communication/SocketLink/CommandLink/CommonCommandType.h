#pragma once

#include <string.h>
#include <stdint.h>

const uint32_t g_commondListSize = 3;

enum CommondTypeList
{
    Link_Commond = 0,
    Link_File,
    Link_EndFile,
};

extern const char *g_commondList[];

CommondTypeList GetCommondType(uint8_t* strMessage);