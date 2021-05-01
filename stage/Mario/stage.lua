--[[
Mario is large stage in code, but not that complex, mario just has lots of moves.

It does use a few more advance things then the Guitarfun stage.
--]]

TEX_BG = loadTexture("bg.png");
TEX_STAGE = loadTexture("stage.png");
TEX_MARIO = loadTexture("mario.png");

cloud1x = math.random(-12, 12)
cloud1y = 4.0
cloud2x = math.random(-12, 12)
cloud2y = 4.0

lastTick = getTime();

mariodir = 1
marioaction = 0.0
mariotime = 0.0
mariox = math.random(-400, 400) / 100
marioy = 2.0
mariostage = math.random(0, 2)

drummerx = math.random(-400, 400) / 100

enemy = {}
enemyCnt = 0

function draw()
	tickDiff = (getTime() - lastTick) / 1000;
	--Handle new enemies
	if hasBit(getInputPress(), 0x3000) then
		enemy[enemyCnt] = {}
		enemy[enemyCnt].x = -6
		enemy[enemyCnt].type = math.random(0, 1)
		enemyCnt = enemyCnt + 1
	end
	
	---Draw back to front due to z-buffering
	
	--back layer
	setPos(6, 3, -1.1, 0, 0, 0);
	drawTexture(TEX_BG, 0, 0.0, 1, 0.5, 6, 3, 255);
	setPos(-6, 3, -1.1, 0, 0, 0);
	drawTexture(TEX_BG, 0, 0.5, 1, 0.5, 6, 3, 255);
	
	setPos( cloud1x, cloud1y,-1.05, 0, 0, 0);
	drawTexture(TEX_STAGE, 0.3125, 0.0, 0.125, 0.125, 1, 1, 255);
	cloud1x = cloud1x - tickDiff / 2
	if cloud1x < -12 then
		cloud1x = 12
		cloud1y = math.random(200, 600) / 100;
	end
	setPos( cloud2x, cloud2y,-1, 0, 0, 0);
	drawTexture(TEX_STAGE, 0.3125, 0.0, 0.125, 0.125, 1, 1, 255);
	cloud2x = cloud2x - tickDiff / 3
	if cloud2x < -12 then
		cloud2x = 12
		cloud2y = math.random(200, 600) / 100;
	end
	--floor
	for i=-7,7,2 do
		setPos(i, 0.0, 0.0, 0, 90, 0);
		drawTexture(TEX_STAGE, 0.5, 0.0, 0.125, 0.125, 1.1, 1.1, 255);
	end
	

	--Center layer
	setPos(drummerx, 3.5,-0.1, 0, 0, 0);
	if getTime(240, 2) == 0 then
		drawTexture(TEX_STAGE, 0.505 + 0.25 * getTime(150, 2), 0.380, 0.24, 0.24, 1.5, 1.5, 255);
	else
		drawTexture(TEX_STAGE, 0.255 + 0.25 * getTime(150, 2), 0.630, 0.24, 0.24, 1.5, 1.5, 255);
	end
	
	setPos(mariox, marioy - 1, 0, 0, 0, 0);
	drawTexture(TEX_STAGE, 0.25 + 0.25 * mariostage, 0.25, 0.25, 0.125, 2.0, 1.0, 255);
	
	setPos(mariox, marioy + 1.0, 0, 0, 0, 0);
	if marioaction == 0 then
		drawTexture(TEX_MARIO, 0.0625 * 11, 0.0, 0.0625, 0.0625, mariodir, 1, 255);
		mariotime = mariotime + tickDiff
		if mariotime > 0.1 then
			mariotime = 0
			--new action
			mariodir = math.random(0, 1); if mariodir == 0 then mariodir = -1 end
			marioaction = math.random(0, 7);
		end
	elseif marioaction == 1 then
		drawTexture(TEX_MARIO, 0.0625 * 12, 0.0, 0.0625, 0.0625, mariodir, 1, 255);
		mariotime = mariotime + tickDiff
		if mariotime > 0.7 then
			mariotime = 0
			marioaction = 0;
		end
	elseif marioaction == 2 then
		mariotime = mariotime + tickDiff
		if mariotime < 0.8 then
			drawTexture(TEX_MARIO, 0.0625 * 6, 0.0, 0.0625, 0.0625, mariodir, 1, 255);
		elseif mariotime < 1.0 then
			drawTexture(TEX_MARIO, 0.0625 * 5, 0.0, 0.0625, 0.0625, mariodir, 1, 255);
			setPos(mariox, marioy + 1.5 + 10 * (1.0 - mariotime), 0.05, 0, 0, 0);
			drawTexture(TEX_STAGE, 0.375, 0.5, 0.125, 0.125, 1, 1, 255);
		elseif mariotime < 1.2 then
			drawTexture(TEX_MARIO, 0.0625 * 5, 0.0, 0.0625, 0.0625, mariodir, 1, 255);
			setPos(mariox, marioy + 1.5, 0.05, 0, 0, 0);
			drawTexture(TEX_STAGE, 0.375, 0.5, 0.125, 0.125, 1 + (mariotime - 1.0) * 20, 1 - (mariotime - 1.0) * 4, 255 - (mariotime - 1.0) * 800);
		elseif mariotime < 1.8 then
			drawTexture(TEX_MARIO, 0.0625 * 5, 0.0, 0.0625, 0.0625, mariodir, 1, 255);
		else
			drawTexture(TEX_MARIO, 0.0625 * 6, 0.0, 0.0625, 0.0625, mariodir, 1, 255);
			mariotime = 0
			marioaction = 0;
		end
	elseif marioaction == 3 then
		mariotime = mariotime + tickDiff
		if mariotime < 1.0 then
			drawTexture(TEX_MARIO, 0.0625 * math.floor(mariotime * 6), 0.09375 * 2, 0.0625, 0.0625, mariodir, 1, 255);
		else
			drawTexture(TEX_MARIO, 0.0625 * 5, 0.0625 * 3, 0.0625, 0.0625, mariodir, 1, 255);
			setPos(mariox + (mariotime - 0.9) * 10 * mariodir, marioy + 1.0, 0.05, 0, 0, 0);
			drawTexture(TEX_MARIO, 0.0625 * (6 + getTime(100, 3)), 0.09375 * 2, 0.0625, 0.0625, mariodir, 1, 128);
			
			if mariox + (mariotime - 1.0) * 10 * mariodir < -12 or mariox + (mariotime - 1.0) * 10 * mariodir > 12 then
				mariotime = 0
				marioaction = 0;
			end
		end
	elseif marioaction == 4 then
		mariotime = mariotime + tickDiff
		if mariotime < 2.0 then
			drawTexture(TEX_MARIO, 0.0625 * math.fmod(math.floor(mariotime * 8), 4), 0.09375 * 4, 0.0625, 0.0625, mariodir, 1, 255);
		else
			drawTexture(TEX_MARIO, 0.0625 * 11, 0.0, 0.0625, 0.0625, mariodir, 1, 255);
			mariotime = 0
			marioaction = 0;
		end
	elseif marioaction == 5 then
		mariotime = mariotime + tickDiff
		if mariotime < 1.5 then
			drawTexture(TEX_MARIO, 0.0625 * (12 + math.fmod(math.floor(mariotime * 8), 3)), 0.09375 * 3, 0.0625, 0.0625, mariodir, 1, 255);
		else
			drawTexture(TEX_MARIO, 0.0625 * 11, 0.0, 0.0625, 0.0625, mariodir, 1, 255);
			mariotime = 0
			marioaction = 0;
		end
	elseif marioaction == 6 then
		mariotime = mariotime + tickDiff
		if mariotime < 2.3 then
			if math.fmod(math.floor(mariotime * 4), 2) == 0 then
				drawTexture(TEX_MARIO, 0.0625 * 8, 0.09375 * 3, 0.0625, 0.0625, mariodir, 1, 255);
			else
				drawTexture(TEX_MARIO, 0.0625 * 5, 0.09375 * 3, 0.0625, 0.0625, mariodir, 1, 255);
			end
		else
			drawTexture(TEX_MARIO, 0.0625 * 11, 0.0, 0.0625, 0.0625, mariodir, 1, 255);
			mariotime = 0
			marioaction = 0;
		end
	elseif marioaction == 7 then
		mariotime = mariotime + tickDiff
		if mariotime < 2.1 then
			drawTexture(TEX_MARIO, 0.0625 * (4 + math.fmod(math.floor(mariotime * 8), 3)), 0.09375 * 4, 0.0625, 0.0625, mariodir, 1, 255);
		else
			drawTexture(TEX_MARIO, 0.0625 * 11, 0.0, 0.0625, 0.0625, mariodir, 1, 255);
			mariotime = 0
			marioaction = 0;
		end
	end
    
	for i=0,enemyCnt-1 do
		enemy[i].x = enemy[i].x + tickDiff * 3
		setPos(enemy[i].x, 1, 0.5, 0, 0, 0);
		
		if getTime(175, 2) == 0 then
			if enemy[i].type == 0 then
				drawTexture(TEX_STAGE, 0.0, 0.5, 0.125, 0.125,-1.0, 1.0, 255);
			else
				drawTexture(TEX_STAGE, 0.125, 0.5, 0.125, 0.125, 1.0, 1.0, 255);
			end
		else
			if enemy[i].type == 0 then
				drawTexture(TEX_STAGE, 0.0, 0.5, 0.125, 0.125, 1.0, 1.0, 255);
			else
				drawTexture(TEX_STAGE, 0.25, 0.5, 0.125, 0.125, 1.0, 1.0, 255);
			end
		end
	end
	if enemyCnt > 0 and enemy[0].x > 6 then
		enemyCnt = enemyCnt - 1
		for i=0,enemyCnt-1 do
			enemy[i] = enemy[i+1]
		end
	end
	
	--Front
	setPos(-6, 3.0, 1.0, 0, 0, 0);
	drawTexture(TEX_STAGE, 0.00, 0.75, 0.25, 0.25, 3, 3, 255);
	setPos( 6, 4.5, 1.0, 0, 0, 0);
	drawTexture(TEX_STAGE, 0.75, 0.625, 0.25, 0.325, 3, 4.5, 255);

	for i=-7,7,2 do
		setPos(i, 5.5, 1.1, 0, 0, 0);
		drawTexture(TEX_STAGE, 0.0, 0.0, 0.25, 0.5, 1, 2, 255);
	end
	
	
	lastTick = getTime();
end
