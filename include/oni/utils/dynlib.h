#pragma once
#include <oni/utils/types.h>

int64_t sys_dynlib_load_prx(char* prxPath);
int64_t sys_dynlib_dlsym(int64_t moduleHandle, const char* functionName, void *destFuncOffset);

