#include <string.h>
#include <stdint.h>

const uint32_t g_commondListSize = 3;

enum CommondTypeList
{
    Link_Commond = 0,
    Link_File,
    Link_EndFile,
};

const char *g_commondList[] = 
{
    "Link-commond",
    "Link-file", 
    "Link-endfile", 
};


CommondTypeList GetCommondType(uint8_t* strMessage)
{
    for (uint32_t i = 0; i < g_commondListSize; ++i) {
        if (strstr((char*)strMessage, g_commondList[i])) {
            return (CommondTypeList)i;
        }
    }

    return (CommondTypeList)-1;
}