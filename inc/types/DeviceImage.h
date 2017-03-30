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

#ifndef DEVICE_IMAGE_H
#define DEVICE_IMAGE_H

#include "DeviceConfig.h"
#include "ManagedString.h"
#include "RefCounted.h"

struct ImageData : RefCounted
{
    uint16_t width;     // Width in pixels
    uint16_t height;    // Height in pixels
    uint8_t data[0];    // 2D array representing the bitmap image
};

/**
  * Class definition for a DeviceImage.
  *
  * An DeviceImage is a simple bitmap representation of an image.
  * n.b. This is a mutable, managed type.
  */
class DeviceImage
{
    ImageData *ptr;     // Pointer to payload data


    /**
      * Internal constructor which provides sanity checking and initialises class properties.
      *
      * @param x the width of the image
      *
      * @param y the height of the image
      *
      * @param bitmap an array of integers that make up an image.
      */
    void init(const int16_t x, const int16_t y, const uint8_t *bitmap);

    /**
      * Internal constructor which defaults to the Empty Image instance variable
      */
    void init_empty();

    public:
    static DeviceImage EmptyImage;    // Shared representation of a null image.

    /**
      * Get current ptr, do not decr() it, and set the current instance to empty image.
      *
      * This is to be used by specialized runtimes which pass ImageData around.
      */
    ImageData *leakData();

    /**
      * Return a 2D array representing the bitmap image.
      */
    uint8_t *getBitmap()
    {
        return ptr->data;
    }

    /**
      * Constructor.
      * Create an image from a specially prepared constant array, with no copying. Will call ptr->incr().
      *
      * @param ptr The literal - first two bytes should be 0xff, then width, 0, height, 0, and the bitmap. Width and height are 16 bit. The literal has to be 4-byte aligned.
      *
      * @code
      * static const uint8_t heart[] __attribute__ ((aligned (4))) = { 0xff, 0xff, 10, 0, 5, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, }; // a cute heart
      * DeviceImage i((ImageData*)(void*)heart);
      * @endcode
      */
    DeviceImage(ImageData *ptr);

    /**
      * Default Constructor.
      * Creates a new reference to the empty DeviceImage bitmap
      *
      * @code
      * DeviceImage i(); //an empty image instance
      * @endcode
      */
    DeviceImage();


    /**
      * Copy Constructor.
      * Add ourselves as a reference to an existing DeviceImage.
      *
      * @param image The DeviceImage to reference.
      *
      * @code
      * DeviceImage i("0,1,0,1,0\n");
      * DeviceImage i2(i); //points to i
      * @endcode
      */
    DeviceImage(const DeviceImage &image);

    /**
      * Constructor.
      * Create a blank bitmap representation of a given size.
      *
      * @param s A text based representation of the image given whitespace delimited numeric values.
      *
      * @code
      * DeviceImage i("0,1,0,1,0\n1,0,1,0,1\n0,1,0,1,0\n1,0,1,0,1\n0,1,0,1,0\n"); // 5x5 image
      * @endcode
      */
    explicit DeviceImage(const char *s);

    /**
      * Constructor.
      * Create a blank bitmap representation of a given size.
      *
      * @param x the width of the image.
      *
      * @param y the height of the image.
      *
      * Bitmap buffer is linear, with 8 bits per pixel, row by row,
      * top to bottom with no word alignment. Stride is therefore the image width in pixels.
      * in where w and h are width and height respectively, the layout is therefore:
      *
      * |[0,0]...[w,o][1,0]...[w,1]  ...  [[w,h]
      *
      * A copy of the image is made in RAM, as images are mutable.
      */
    DeviceImage(const int16_t x, const int16_t y);

    /**
      * Constructor.
      * Create a bitmap representation of a given size, based on a given buffer.
      *
      * @param x the width of the image.
      *
      * @param y the height of the image.
      *
      * @param bitmap a 2D array representing the image.
      *
      * @code
      * const uint8_t heart[] = { 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, }; // a cute heart
      * DeviceImage i(10,5,heart);
      * @endcode
      */
    DeviceImage(const int16_t x, const int16_t y, const uint8_t *bitmap);

    /**
      * Destructor.
      *
      * Removes buffer resources held by the instance.
      */
    ~DeviceImage();

    /**
      * Copy assign operation.
      *
      * Called when one DeviceImage is assigned the value of another using the '=' operator.
      *
      * Decrement our reference count and free up the buffer as necessary.
      *
      * Then, update our buffer to refer to that of the supplied DeviceImage,
      * and increase its reference count.
      *
      * @param s The DeviceImage to reference.
      *
      * @code
      * const uint8_t heart[] = { 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, }; // a cute heart
      * DeviceImage i(10,5,heart);
      * DeviceImage i1();
      * i1 = i; // i1 now references i
      * @endcode
      */
    DeviceImage& operator = (const DeviceImage& i);


    /**
      * Equality operation.
      *
      * Called when one DeviceImage is tested to be equal to another using the '==' operator.
      *
      * @param i The DeviceImage to test ourselves against.
      *
      * @return true if this DeviceImage is identical to the one supplied, false otherwise.
      *
      * @code
      * DeviceDisplay display;
      * DeviceImage i();
      * DeviceImage i1();
      *
      * if(i == i1) //will be true
      *     display.scroll("true");
      * @endcode
      */
    bool operator== (const DeviceImage& i);

    /**
      * Resets all pixels in this image to 0.
      *
      * @code
      * DeviceImage i("0,1,0,1,0\n1,0,1,0,1\n0,1,0,1,0\n1,0,1,0,1\n0,1,0,1,0\n"); // 5x5 image
      * i.clear();
      * @endcode
      */
    void clear();

    /**
      * Sets the pixel at the given co-ordinates to a given value.
      *
      * @param x The co-ordinate of the pixel to change.
      *
      * @param y The co-ordinate of the pixel to change.
      *
      * @param value The new value of the pixel (the brightness level 0-255)
      *
      * @return DEVICE_OK, or DEVICE_INVALID_PARAMETER.
      *
      * @code
      * DeviceImage i("0,1,0,1,0\n1,0,1,0,1\n0,1,0,1,0\n1,0,1,0,1\n0,1,0,1,0\n"); // 5x5 image
      * i.setPixelValue(0,0,255);
      * @endcode
      *
      * @note all coordinates originate from the top left of an image.
      */
    int setPixelValue(int16_t x , int16_t y, uint8_t value);

    /**
      * Retrieves the value of a given pixel.
      *
      * @param x The x co-ordinate of the pixel to read. Must be within the dimensions of the image.
      *
      * @param y The y co-ordinate of the pixel to read. Must be within the dimensions of the image.
      *
      * @return The value assigned to the given pixel location (the brightness level 0-255), or DEVICE_INVALID_PARAMETER.
      *
      * @code
      * DeviceImage i("0,1,0,1,0\n1,0,1,0,1\n0,1,0,1,0\n1,0,1,0,1\n0,1,0,1,0\n"); // 5x5 image
      * i.getPixelValue(0,0); //should be 0;
      * @endcode
      */
    int getPixelValue(int16_t x , int16_t y);

    /**
      * Replaces the content of this image with that of a given 2D array representing
      * the image.
      *
      * @param x the width of the image. Must be within the dimensions of the image.
      *
      * @param y the width of the image. Must be within the dimensions of the image.
      *
      * @param bitmap a 2D array representing the image.
      *
      * @return DEVICE_OK on success, or DEVICE_INVALID_PARAMETER.
      *
      * @code
      * const uint8_t heart[] = { 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, }; // a cute heart
      * DeviceImage i();
      * i.printImage(0,0,heart);
      * @endcode
      *
      * @note all coordinates originate from the top left of an image.
      */
    int printImage(int16_t x, int16_t y, const uint8_t *bitmap);

    /**
      * Pastes a given bitmap at the given co-ordinates.
      *
      * Any pixels in the relevant area of this image are replaced.
      *
      * @param image The DeviceImage to paste.
      *
      * @param x The leftmost X co-ordinate in this image where the given image should be pasted. Defaults to 0.
      *
      * @param y The uppermost Y co-ordinate in this image where the given image should be pasted. Defaults to 0.
      *
      * @param alpha set to 1 if transparency clear pixels in given image should be treated as transparent. Set to 0 otherwise.  Defaults to 0.
      *
      * @return The number of pixels written.
      *
      * @code
      * const uint8_t heart[] = { 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, }; // a cute heart
      * DeviceImage i(10,5,heart); // a big heart
      * i.paste(i, -5, 0); // a small heart
      * @endcode
      */
    int paste(const DeviceImage &image, int16_t x = 0, int16_t y = 0, uint8_t alpha = 0);

     /**
       * Prints a character to the display at the given location
       *
       * @param c The character to display.
       *
       * @param x The x co-ordinate of on the image to place the top left of the character. Defaults to 0.
       *
       * @param y The y co-ordinate of on the image to place the top left of the character. Defaults to 0.
       *
       * @return DEVICE_OK on success, or DEVICE_INVALID_PARAMETER.
       *
       * @code
       * DeviceImage i(5,5);
       * i.print('a');
       * @endcode
       */
    int print(char c, int16_t x = 0, int16_t y = 0);

    /**
      * Shifts the pixels in this Image a given number of pixels to the left.
      *
      * @param n The number of pixels to shift.
      *
      * @return DEVICE_OK on success, or DEVICE_INVALID_PARAMETER.
      *
      * @code
      * const uint8_t heart[] = { 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, }; // a cute heart
      * DeviceImage i(10,5,heart); // a big heart
      * i.shiftLeft(5); // a small heart
      * @endcode
      */
    int shiftLeft(int16_t n);

    /**
      * Shifts the pixels in this Image a given number of pixels to the right.
      *
      * @param n The number of pixels to shift.
      *
      * @return DEVICE_OK on success, or DEVICE_INVALID_PARAMETER.
      *
      * @code
      * const uint8_t heart[] = { 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, }; // a cute heart
      * DeviceImage i(10,5,heart); // a big heart
      * i.shiftLeft(5); // a small heart
      * i.shiftRight(5); // a big heart
      * @endcode
      */
    int shiftRight(int16_t n);

    /**
      * Shifts the pixels in this Image a given number of pixels to upward.
      *
      * @param n The number of pixels to shift.
      *
      * @return DEVICE_OK on success, or DEVICE_INVALID_PARAMETER.
      *
      * @code
      * const uint8_t heart[] = { 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, }; // a cute heart
      * DeviceImage i(10,5,heart);
      * i.shiftUp(1);
      * @endcode
      */
    int shiftUp(int16_t n);

    /**
      * Shifts the pixels in this Image a given number of pixels to downward.
      *
      * @param n The number of pixels to shift.
      *
      * @return DEVICE_OK on success, or DEVICE_INVALID_PARAMETER.
      *
      * @code
      * const uint8_t heart[] = { 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, }; // a cute heart
      * DeviceImage i(10,5,heart);
      * i.shiftDown(1);
      * @endcode
      */
    int shiftDown(int16_t n);

    /**
      * Gets the width of this image.
      *
      * @return The width of this image.
      *
      * @code
      * const uint8_t heart[] = { 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, }; // a cute heart
      * DeviceImage i(10,5,heart);
      * i.getWidth(); // equals 10...
      * @endcode
      */
    int getWidth() const
    {
        return ptr->width;
    }

    /**
      * Gets the height of this image.
      *
      * @return The height of this image.
      *
      * @code
      * const uint8_t heart[] = { 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, }; // a cute heart
      * DeviceImage i(10,5,heart);
      * i.getHeight(); // equals 5...
      * @endcode
      */
    int getHeight() const
    {
        return ptr->height;
    }

    /**
      * Gets number of bytes in the bitmap, ie., width * height.
      *
      * @return The size of the bitmap.
      *
      * @code
      * const uint8_t heart[] = { 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, }; // a cute heart
      * DeviceImage i(10,5,heart);
      * i.getSize(); // equals 50...
      * @endcode
      */
    int getSize() const
    {
        return ptr->width * ptr->height;
    }

    /**
      * Converts the bitmap to a csv ManagedString.
      *
      * @code
      * const uint8_t heart[] = { 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, }; // a cute heart
      * DeviceImage i(10,5,heart);
      * uBit.serial.printString(i.toString()); // "0,1,0,1,0,0,0,0,0,0\n..."
      * @endcode
      */
    ManagedString toString();

    /**
      * Crops the image to the given dimensions.
      *
      * @param startx the location to start the crop in the x-axis
      *
      * @param starty the location to start the crop in the y-axis
      *
      * @param width the width of the desired cropped region
      *
      * @param height the height of the desired cropped region
      *
      * @code
      * const uint8_t heart[] = { 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, }; // a cute heart
      * DeviceImage i(10,5,heart);
      * i.crop(0,0,2,2).toString() // "0,1\n1,1\n"
      * @endcode
      */
    DeviceImage crop(int startx, int starty, int finx, int finy);

    /**
      * Check if image is read-only (i.e., residing in flash).
      */
    bool isReadOnly();

    /**
      * Create a copy of the image bitmap. Used particularly, when isReadOnly() is true.
      *
      * @return an instance of DeviceImage which can be modified independently of the current instance
      */
    DeviceImage clone();
};

#endif
