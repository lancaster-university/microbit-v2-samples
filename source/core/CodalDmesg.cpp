#include "CodalDmesg.h"

#if DEVICE_DMESG_BUFFER_SIZE > 0

#include <cstdarg>
#include "CodalCompat.h"

CodalLogStore codalLogStore;

using namespace codal;

static void logwrite(const char *msg);

static void logwriten(const char *msg, int l)
{
    const int jump = sizeof(codalLogStore.buffer) / 4;
    if (codalLogStore.ptr >= sizeof(codalLogStore.buffer) - jump)
    {
        codalLogStore.ptr -= jump;
        memmove(codalLogStore.buffer, codalLogStore.buffer + jump, codalLogStore.ptr);
    }
    if (l + codalLogStore.ptr >= sizeof(codalLogStore.buffer))
    {
        logwrite("DMESG line too long!\n");
        return;
    }
    memcpy(codalLogStore.buffer + codalLogStore.ptr, msg, l);
    codalLogStore.ptr += l;
    codalLogStore.buffer[codalLogStore.ptr] = 0;
}

static void logwrite(const char *msg)
{
    logwriten(msg, strlen(msg));
}

static void writeNum(char *buf, uint32_t n, bool full)
{
    int i = 0;
    int sh = 28;
    while (sh >= 0)
    {
        int d = (n >> sh) & 0xf;
        if (full || d || sh == 0 || i)
        {
            buf[i++] = d > 9 ? 'A' + d - 10 : '0' + d;
        }
        sh -= 4;
    }
    buf[i] = 0;
}

static void logwritenum(uint32_t n, bool full, bool hex)
{
    char buff[20];

    if (hex)
    {
        writeNum(buff, n, full);
        logwrite("0x");
    }
    else
    {
        itoa(n, buff);
    }

    logwrite(buff);
}

void codal_dmesg(const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    codal_vdmesg(format, arg);
    va_end(arg);
}

void codal_vdmesg(const char *format, va_list ap)
{
    const char *end = format;

    while (*end)
    {
        if (*end++ == '%')
        {
            logwriten(format, end - format - 1);
            uint32_t val = va_arg(ap, uint32_t);
            switch (*end++)
            {
            case 'd':
                logwritenum(val, false, false);
                break;
            case 'x':
                logwritenum(val, false, true);
                break;
            case 'p':
            case 'X':
                logwritenum(val, true, true);
                break;
            case 's':
                logwrite((char *)(void *)val);
                break;
            case '%':
                logwrite("%");
                break;
            default:
                logwrite("???");
                break;
            }
            format = end;
        }
    }
    logwriten(format, end - format);
    logwrite("\n");
}

#endif
