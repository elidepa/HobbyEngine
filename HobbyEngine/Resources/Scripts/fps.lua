
frameCount = 0
timeCount = 0

function update(deltaTime)
	frameCount = frameCount + 1
	timeCount = timeCount + deltaTime

	if (timeCount > 1000)
	then
		local fps = frameCount / timeCount * 1000.0
		local text = string.format("FPS: %.2f", fps)
		frameCount = 0
		timeCount = 0
		updateText(text)
	end

end
