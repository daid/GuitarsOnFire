Some notes about the source.

First, licencing, everything is GPL or GPL compattible. I used a few libraries not included in devkitpro, I included all source for those.
They are:
-lua
-libtremmor
-libpng
-pngu

If you want to build the sources, it lacks a Makefile, this is because I used code::blocks to build everything. The source package includes my code::blocks project with setttings.

Some documentation of functions is put in the header files, but that's all there is.

Some files have a *_gekko.c as well as a *_pc.c, these contain the same functions, only implemented for the Wii or win32/linux. So you can build everything for the PC platform as well as for the Wii.

In the source package is a "libogc_patch.patch" this patch need to be applied to libogc. Without it GoF won't compile, and if it compiles it won't support drums and world tour guitars!