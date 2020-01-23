
counter = 4100;

function update(deltaTime)
	counter =  counter + deltaTime
	if (counter >= 5000) 
	then
		activateParticles()
		counter = 0
	end

	local x, y, z = getPosition()

	local posCycle = math.cos(counter / 5000 * 6.283)

	-- deltaY is the distance from current y pos to the destination y, which is in the range [0.5,1.5]
	local deltaY = (posCycle * 0.5 + 1.0) - y

	move(0, deltaY, 0)
end
