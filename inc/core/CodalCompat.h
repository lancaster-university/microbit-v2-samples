/*
The MIT License (MIT)

Copyright (c) 2016 British Broadcasting Corporation.
This software is provided by Lancaster University by arrangement with the BBC.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

/**
  * This file contains functions used to maintain compatability and portability.
  * It also contains constants that are used elsewhere in the DAL.
  */

#ifndef DEVICE_COMPAT_H
#define DEVICE_COMPAT_H

#include "DeviceConfig.h"

#ifndef PI
#define PI 3.14159265359
#endif

namespace codal
{
    /**
      * Determines the smallest of the two numbers
      *
      * @param a the first number
      *
      * @param b the second number
      *
      * @return The smallest of the two given values.
      */
    inline int min(int a, int b)
    {
        return (a < b ? a : b);
    }

    /**
      * Determines the largest of the two numbers
      *
      * @param a the first number
      *
      * @param b the second number
      *
      * @return The larger of the two given values.
      */
    inline int max(int a, int b)
    {
        return (a > b ? a : b);
    }

    /**
      * Sets a given area of memory to zero.
      *
      * @param a the pointer to the beginning of the memory to clear
      *
      * @param b the number of bytes to clear.
      */
    inline void *memclr(void *a, size_t b)
    {
        return memset(a,0,b);
    }

    /**
      * Determines if the given character is a printable ASCII/UTF8 decimal digit (0..9).
      *
      * @param c the character to check
      *
      * @return true if the character is a digit, false otherwise.
      */
    inline bool isdigit(char c)
    {
        return (c > 47 && c < 58);
    }

    /**
      * Extracts the upper 8 bits of a 16 bit integer
      *
      * @param val the 16 bit value.
      *
      * @return the upper 8 bits of the given val.
      */
    inline uint8_t high(uint16_t val)
    {
        return val >> 8;
    }

    /**
      * Extracts the lower 8 bits of a 16 bit integer
      *
      * @param val the 16 bit value.
      *
      * @return the lower 8 bits of the given val.
      */
    inline uint8_t low(uint16_t val)
    {
        return val & 0xFF;
    }

    /**
      * Performs an in buffer reverse of a given char array.
      *
      * @param s the string to reverse.
      *
      * @return DEVICE_OK, or DEVICE_INVALID_PARAMETER.
      */
    int string_reverse(char *s);

    /**
      * Converts a given integer into a string representation.
      *
      * @param n The number to convert.
      *
      * @param s A pointer to the buffer where the resulting string will be stored.
      *
      * @return DEVICE_OK, or DEVICE_INVALID_PARAMETER.
      */
    int itoa(int n, char *s);

}

#endif
