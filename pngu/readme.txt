********************************************************************************************

PNGU Version : 0.2a

Coder : frontier

More info : http://frontier-dev.net

********************************************************************************************

PNGU is a library for handling png images in Wii/GC based in libpng 1.2.29.


Current version features:

- It's based in libpng 1.2.29

- Handles images of 8 and 16 bits per channel.

- Handles images in RGB, RGBA, grayscale and grayscale + alpha formats.

- Reads image dimensions, pixel format, background color and transparent colors list.

- Converts images to YCbYCr, linear RGB565, linear RGBA8, 4x4 RGB565, 4x4 RGB5A3 and 4x4 RGBA8 formats.

- Saves YCbYCr images in png RGB8 format.

- Handles images stored in memory or in devoptab devices (SD, Gecko SD, etc...).

- It's ready to be used in multithreaded applications.


Current version limitations:

- Doesn't handle images of 1, 2 or 4 bits per channel.

- Doesn't handle paletized images.



NOTES:

- When converting images it's the user's responsability to allocate enough memory to store the output images. The required buffer size depends of image dimensions and pixel format. For YCbYCr, RGB565, 4x4 RGB565 and 4x4 RGB5A3 formats the required buffer size in bytes is width * height * 2. For RGBA8 and 4x4 RGBA8 formats the required buffer size in bytes is width * height * 4. Both width and height are in pixels.

- Memory allocated by the user must be always freed by the user. Memory allocated by PNGU must be freed by PNGU, so it's important that you always call PNGU_ReleaseImageContext when you no longer need the image context.

- Validity of the transparent colors list returned by PNGU_GetImageProperties is only warranted till the next call to a PNGU function. So, if the list is to be preserved the user must copy it. Never explicitly free the list, PNGU takes care or it when PNGU_ReleaseImageContext is called.

- Only can be converted to 4x4 formats those images which width and height are both multiple of 4. Only can be converted to YCbYCr format those images which width is multiple of 2.

- If a FAT device is to be used (SD, Gecko SD), libfat must be initialized before calling PNGU functions.