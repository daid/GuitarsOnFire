
function reset()
	map = {}
	for x=0,11 do
		for y=0,22 do
			map[x+y*12] = 0;
		end
	end
	
	shapeX = 6
	shapeY = 20
	shapeNum = math.random(0, 18);
	lastTime = getTime();
end
reset();

TL = {-1, 1};
TC = { 0, 1};
TR = { 1, 1};
ML = {-1, 0};
MR = { 1, 0};
MR2 ={ 2, 0};
BL = {-1,-1};
BC = { 0,-1};
BR = { 1,-1};
B2C ={ 0, 2};

shapes = {}
shapes[0] = {7, TL,TC,MR};
shapes[1] = {8, TR,TC,ML};
shapes[2] = {9, ML,MR,BC}; 
shapes[3] = {3, TL,TC,ML};
shapes[4] = {12,ML,BL,MR};
shapes[5] = {15,ML,BR,MR};
shapes[6] = {18,ML,MR,MR2};
shapes[7] = {0, TC,ML,BL};
shapes[8] = {1, TC,MR,BR};
shapes[9] = {10,TC,MR,BC};
shapes[10] = {11,TC,ML,MR};
shapes[11] = {2, TC,ML,BC};
shapes[12] = {13,TC,BC,BR};
shapes[13] = {14,TR,ML,MR};
shapes[14] = {4, TL,TC,BC};
shapes[15] = {16,TR,TC,BC};
shapes[16] = {17,TL,MR,ML};
shapes[17] = {5, TC,BC,BL};
shapes[18] = {6, TC,BC,B2C};

lineCount = 0
colors = {0xFF0000, 0x00FF00, 0xFFFF00, 0x0000FF, 0xFF00FF, 0x00FFFF, 0xFFFFFF};

function mapTaken(x, y)
	if x < 0 then return true; end
	if y < 0 then return true; end
	if x > 11 then return true; end
	if y > 22 then return true; end
	if map[x + y * 12] ~= 0 then return true; end
	return false;
end
function mapSet(x, y, num)
	map[x + y * 12] = num
end

function shapeFits(num, x, y)
	if mapTaken(x, y) then return false; end
	for i=2,4 do
		if mapTaken(x + shapes[num][i][1], y + shapes[num][i][2]) then return false; end
	end
	return true;
end

function tick()
	if shapeFits(shapeNum, shapeX, shapeY - 1) then
		shapeY = shapeY - 1;
	else
		col = colors[math.random(1, 7)];
		mapSet(shapeX, shapeY, col);
		for i=2,4 do
			mapSet(shapeX + shapes[shapeNum][i][1], shapeY + shapes[shapeNum][i][2], col);
		end
		shapeNum = math.random(0, 18);
		shapeX = math.random(3, 9);
		shapeY = 20;
		
		if mapTaken(shapeX, shapeY) then
			reset();
		else
			for y=0,22 do
				fill = true;
				while fill do
					for x=0,11 do
						if map[x+y*12] == 0 then fill = false; end
					end
					if fill then
						lineCount = lineCount + 1
						for y2=y,21 do
							for x=0,11 do
								map[x + y2*12] = map[x + (y2+1)*12];
							end
						end
						for x=0,11 do
							map[x + 22*12] = 0;
						end
					end
				end
			end
		end
	end
end

function draw()
	if getTime() - lastTime > 200 then
		lastTime = lastTime + 200;
		tick();
	end

	for i=1,lineCount do
		setPos(-7, i - 8, -20, 0, 0, 0);
		drawQuad(0.4, 0.4, 0xFFFFFF, 255);
	end

	for x=0,11 do
		for y=0,22 do
			setPos(x - 5.5, y - 8, -20, 0, 0, 0);
			if map[x + y * 12] == 0 then
				col = 0x202020
			else
				col = map[x + y * 12]
			end
			drawQuad(0.4, 0.4, col, 255);
		end
	end
	
	setPos(shapeX - 5.5, shapeY - 8, -20, 0, 0, 0);
	drawQuad(0.4, 0.4, 0xFFFFFF, 255);
	for i=2,4 do
		setPos(shapeX + shapes[shapeNum][i][1] - 5.5, shapeY + shapes[shapeNum][i][2] - 8, -20, 0, 0, 0);
		drawQuad(0.4, 0.4, 0xFFFFFF, 255);
	end

	if hasBit(getInputPress(), 0x0010) then --green key
		if shapeFits(shapeNum, shapeX - 1, shapeY) then
			shapeX = shapeX - 1;
		end
	end
	if hasBit(getInputPress(), 0x0020) then	--red key
		if shapeFits(shapes[shapeNum][1], shapeX, shapeY) then
			shapeNum = shapes[shapeNum][1];
		end
	end
	if hasBit(getInputPress(), 0x0040) then	--yellow key
		if shapeFits(shapeNum, shapeX + 1, shapeY) then
			shapeX = shapeX + 1;
		end
	end
end
