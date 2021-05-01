#!/bin/sh

BASE_PATH=`pwd`
PACKAGE_PATH=$BASE_PATH/package

echo Packaging Guitars on Fire at $PACKAGE_PATH
rm -rf $PACKAGE_PATH
mkdir -p $PACKAGE_PATH/apps/GuitarsOnFire/
mkdir -p $PACKAGE_PATH/GuitarsOnFire/sfx
mkdir -p $PACKAGE_PATH/GuitarsOnFire/gfx
mkdir -p $PACKAGE_PATH/GuitarsOnFire/stage
mkdir -p $PACKAGE_PATH/GuitarsOnFire/songs

cp $BASE_PATH/sfx/*.ogg $PACKAGE_PATH/GuitarsOnFire/sfx/
cp $BASE_PATH/gfx/*.png $PACKAGE_PATH/GuitarsOnFire/gfx/
cp $BASE_PATH/gfx/*.raw $PACKAGE_PATH/GuitarsOnFire/gfx/

cp boot.dol $PACKAGE_PATH/apps/GuitarsOnFire/
cp icon.png $PACKAGE_PATH/apps/GuitarsOnFire/
cp meta.xml $PACKAGE_PATH/apps/GuitarsOnFire/

cp StagePreview.exe $PACKAGE_PATH/GuitarsOnFire/
cp sdl.dll $PACKAGE_PATH/GuitarsOnFire/
cp zlib1.dll $PACKAGE_PATH/GuitarsOnFire/
cp readme.txt $PACKAGE_PATH/GuitarsOnFire/
cp COPYING $PACKAGE_PATH/GuitarsOnFire/

for STAGE in $BASE_PATH/stage/*; do
	STAGE=`basename "$STAGE"`
	echo "Copying stage: $STAGE"
	mkdir -p "$PACKAGE_PATH/GuitarsOnFire/stage/$STAGE/"
	cp "$BASE_PATH/stage/$STAGE/stage.lua" "$PACKAGE_PATH/GuitarsOnFire/stage/$STAGE/"
	cp "$BASE_PATH/stage/$STAGE/"*.png "$PACKAGE_PATH/GuitarsOnFire/stage/$STAGE/"
done

for SONG in $BASE_PATH/songs/*; do
	SONG=`basename "$SONG"`
	echo "Copying song: $SONG"
	mkdir -p "$PACKAGE_PATH/GuitarsOnFire/songs/$SONG/"
	cp "$BASE_PATH/songs/$SONG/"*.* "$PACKAGE_PATH/GuitarsOnFire/songs/$SONG/"
done
