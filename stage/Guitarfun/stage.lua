--[[
For the lua language look at www.lua.org

GuitarsOnFire functions documentation:
--------------------------------
setPos(x,y,z, yaw, pitch, roll)
	Set the next drawing location, in 3D space. 0,0,0 is the center of the stage, near the end of the neck in 1 player mode.
	
	example: everywhere.
--------------------------------
drawQuad(width, height, color, alpha)
	draws a simple 1 colored square, at te location set with setPos
	
	example: guitarfun stage, floor.
--------------------------------
drawTexture(textureNum, texCoordX, texCoordY, texCoordw, texCoordH, width, height, alpha)
	draws a textured quare

	example: everywhere
--------------------------------
getTime()
	returns the time in miliseconds
getTime(mod)
	returns the time in miliseconds with modulo [mod] (remainder of after dividing with [mod])
getTime(div, mod)
	returns the time in miliseconds divided by [div], and then modulo [mod]
	usefull for animations
	
	example: animation of guitarfun stage, and mario stage
--------------------------------
getInput()
	returns the input for all attached controlers
	(see inputmasks)
getInput(n)
	returns the input for player [n]
getInputPressed()
	returns the input for all attached controlers, but only if just pressed down this tick.
getInputPressed(n)
	returns the input for player [n], but only if just pressed down this tick.
getInputAngle(n)
	returns the 'pitch' of the guitar.
getInputWhammy(n)
	returns the position of the whammy bar. (0 to 1, where 1 is pressed down)
getInputTouch(n)
	returns the touched values of the slider bad (if available)
	
	example: 'enemies' in mario stage, and hand movement of GuitarsOnFire stage
--------------------------------
hasBit(n, m)
	returns true if [n] contains any of the bits found in [m]
	
	example: used in combination with getInput* functions, so same examples.
--------------------------------
getPlayerInfo(n)
	returns a table with player info, containing the following fields:
		playing, score, streak, bestStreak, difficulty, quality, leftyFlip
		
	example: GuitarsOnFire stage uses this for the flames on the guitar, counting players and the rocking hands.
--------------------------------

Input masks:
0x0001 : guitar attached
0x0002 : drums attached
0x0004 : classic controller attached
0x0010 : green fret
0x0020 : red fret
0x0040 : yellow fret
0x0080 : blue fret
0x0100 : orange fret
0x1000 : strum up
0x2000 : strum down
0x4000 : menu button

TouchBar masks
0x1000 : Touchbar available
0x0001 : Touchbar green touched
0x0002 : Touchbar red touched
0x0004 : Touchbar yellow touched
0x0008 : Touchbar blue touched
0x0010 : Touchbar orange touched

--------------------------------
--]]


--[[ Guitarfun is a very basic stage, it should be easy to base your own stages of this one. --]]

--load the textures
TEX_CREW = loadTexture("crew.png");
TEX_CREW2 = loadTexture("crew2.png");
TEX_PUBLIC = loadTexture("public.png");
TEX_SPEAKER = loadTexture("speaker.png");

--Some startup initalization, simply choose some random people on stage by choosing different textures.
crew1 = math.random(0, 1);
crew2 = math.random(0, 1);
crew3 = math.random(0, 1);
if crew1 == 0 then crew1 = TEX_CREW else crew1 = TEX_CREW2 end
if crew2 == 0 then crew2 = TEX_CREW else crew2 = TEX_CREW2 end
if crew3 == 0 then crew3 = TEX_CREW else crew3 = TEX_CREW2 end

function draw()
	--Draw the gray stage platform
	setPos(0,2.0,-2.0, 0, 90, 0);
    drawQuad(8, 2.5, 0x808080, 255);
    setPos(0, 1.0,0.5, 0, 0, 0);
    drawQuad(8, 1, 0x404040, 255);

	--draw the speakers
    setPos(-7, 4.0, -3, 45, 0, 0);
    drawTexture(TEX_SPEAKER, 0, 0, 1, 1, 1.0, 2.0, 255);
    setPos( 7, 4.0, -3,-45, 0, 0);
    drawTexture(TEX_SPEAKER, 0, 0, 1, 1, 1.0, 2.0, 255);
    
	--draw the band
    setPos(-4, 4.0, 0, 0, 0, 0);
    drawTexture(crew1, 0.00, 0.25 * getTime(150, 4), 0.25, 0.25, 2, 2, 255);
    setPos( 0, 4.0,-2, 0, 0, 0);
    drawTexture(crew2, 0.25, 0.25 * getTime(150, 4), 0.25, 0.25, 2, 2, 255);
    setPos( 4, 4.0, 0, 0, 0, 0);
    drawTexture(crew3, 0.50, 0.25 * getTime(150, 4), 0.25, 0.25, 2, 2, 255);

	--draw the crowd
    setPos( 5,1,2.5, 0, -20,0);
    drawTexture(TEX_PUBLIC, 0, 0.5 * getTime(200, 2), 1.0, 0.5, 5, 2.5, 255);
    setPos(-5,1,2.5, 0, -20,0);
    drawTexture(TEX_PUBLIC, 0, 0.5 * getTime(200, 2), 1.0, 0.5, 5, 2.5, 255);
end
