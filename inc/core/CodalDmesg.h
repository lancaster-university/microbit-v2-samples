#ifndef CODAL_DMESG_H
#define CODAL_DMESG_H

#include "DeviceConfig.h"
#include <cstdarg>

#if CONFIG_ENABLED(DEVICE_DMESG)

struct CodalLogStore
{
    uint32_t ptr;
    char buffer[4096];
};
extern CodalLogStore codalLogStore;

void codal_dmesg(const char *format, ...);
void codal_vdmesg(const char *format, std::va_list ap);

#define DMESG codal_dmesg

#else

#define DMESG_NOOP ((void)0)
#define DMESG(...) DMESG_NOOP

#endif

#endif
