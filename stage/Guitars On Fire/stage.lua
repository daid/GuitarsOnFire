--[[

GuitarsOnFire is a pretty complex stage,
it takes in account the number of players and acts on the buttons.
Also looks at the streak and quality of your play to show the flames and rock hands.

--]]

TEX_GUITAR = loadTexture("guitar.png");
TEX_HANDS  = loadTexture("hands.png");

angle = {}
for player=0,4 do
	angle[player] = 0
end

function getAngle(player)
	--Angle goes from -180 to 180, where 180 and -180 are straight up, change that to 0 to 360 so we don't flip around when pointing just over the top.
	a = getInputAngle(player);
	if a < 0 then a = a + 360 end
	return a - 90;
end

function draw()
	playerCount = 0;
	rockhandsize = getTime(400) / 200
	if rockhandsize > 1 then rockhandsize = 2 - rockhandsize end
	rockhandsize = 0.7 + 0.3 * rockhandsize
	
	info = {}
	for player=0,4 do
		info[player] = getPlayerInfo(player);
		if info[player].playing then
			playerCount = playerCount + 1
		end
	end
	
	if playerCount == 1 then
		xoffset = 0
		xinc = 0
		zoffset = 2
	end
	if playerCount == 2 then
		xoffset = -5
		xinc = 8
		zoffset = 0
	end
	if playerCount == 3 then
		xoffset = -8
		xinc = 7
		zoffset = -2
	end
	if playerCount == 4 then
		xoffset = -10
		xinc = 6
		zoffset = -5
	end
	if playerCount == 5 then
		xoffset = -13
		xinc = 6
		zoffset = -8
	end
	for player=0,4 do
		if info[player].playing then
			fire = (info[player].streak / 40) * 255
			if (fire > 255) then
				fire = 255
			end
			rockhandalpha = ((info[player].quality - 500) / 500) * 255
			if rockhandalpha > 255 then rockhandalpha = 255 end
			if rockhandalpha < 0 then rockhandalpha = 0 end
			angle[player] = angle[player] * 0.8 + (getAngle(player)) * 0.2
			
			x = math.cos(math.rad(angle[player]))
			y = math.sin(math.rad(angle[player]))
			
			setPos(xoffset + x*1.3 - y * 0.5,y*1.3 + x*0.5 +4,zoffset, 0,0, angle[player]);
			drawTexture(TEX_GUITAR, 0,0.5, 1,0.5, 3,1.5, 255);

			drawTexture(TEX_GUITAR, 0,  0, 1,0.5, 3,1.5, fire);
			
			pos = -1
			posCount = 0
			input = getInput(player)
			if hasBit(input, 0x0010) then pos = ((pos * posCount) + 0) / (posCount + 1); posCount = posCount + 1; end
			if hasBit(input, 0x0020) then pos = ((pos * posCount) + 1) / (posCount + 1); posCount = posCount + 1; end
			if hasBit(input, 0x0040) then pos = ((pos * posCount) + 2) / (posCount + 1); posCount = posCount + 1; end
			if hasBit(input, 0x0080) then pos = ((pos * posCount) + 3) / (posCount + 1); posCount = posCount + 1; end
			if hasBit(input, 0x0100) then pos = ((pos * posCount) + 4) / (posCount + 1); posCount = posCount + 1; end
			setPos(xoffset + x*(2.2+pos*0.1),y*(2.2+pos*0.1)+4,zoffset+0.05, 0,0, angle[player]);
			drawTexture(TEX_HANDS, 0.5,0, 0.5, 1,-0.6,0.6, 255);

			pos = 0
			if hasBit(input, 0x1000) then pos =-1 end
			if hasBit(input, 0x2000) then pos = 1 end
			pos = pos - 0.5
			setPos(xoffset + x*(-0.25) + y*pos*0.3,y*(-0.25)+4 - x*pos*0.3,zoffset+0.05, 0,0, angle[player]+180);
			drawTexture(TEX_HANDS, 0.5,0, 0.5, 1, 0.6,0.6, 255);

			setPos(xoffset - 1.5, 2 + rockhandsize, zoffset+0.1, 0,0, 0);
			drawTexture(TEX_HANDS, 0,0, 0.5, 1,-rockhandsize, rockhandsize, rockhandalpha);
			setPos(xoffset + 1.5, 2 + rockhandsize, zoffset+0.1, 0,0, 0);
			drawTexture(TEX_HANDS, 0,0, 0.5, 1, rockhandsize, rockhandsize, rockhandalpha);

			xoffset = xoffset + xinc
			zoffset = zoffset + 0.1
		end
	end
end
