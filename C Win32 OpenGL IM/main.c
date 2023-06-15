#include <stdio.h>
#include <stdlib.h>

#include "window.h"
#include "graphics.h"
#include "game.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iCmdShow){
	
	// window
	WindowHandle windowHandle;
	if(!windowSetup(&windowHandle, hInstance, iCmdShow))
		return 0;
	
	// game
	Game game;
	if(gameSetup(&game)){
		printf("Game allocation failure\n");
		return 0;
	}
	int (*updateFuncs[2])(Input *input, Game *game) = { gameUpdate, menuUpdate };
	
	// display
	unsigned int texture = textureSetup("textures.bmp");
	DrawData draw;
	if(drawSetup(&game, &draw, texture)){
		printf("DrawData allocation failure\n");
		return 0;
	}
	void (*drawFuncs[2])(DrawData *draw) = { drawGame, drawMenu };
	
	// input
	Input input;
	inputSetup(&input);
	
	// loop
	float frameRate = 120.f;
	float framePeriod = 1000.f / frameRate;
	int loop = 1;
	int firstFrame = 2;
	int resizeFrame = 1;
	int message = 0;
	while(loop){
		
		// window update
		loop = windowUpdate(&message, &windowHandle, &resizeFrame, &input.toggleMenu, &input.select, &input.flag, &draw.windowSize.x, &draw.windowSize.y, &input.selectPos.x, &input.selectPos.y);
		
		// game update
		if(input.select || input.flag)
			gameCursorPos(&game, &input, &draw);
		int update = updateFuncs[game.isPaused](&input, &game);
		if(update == -1)
			loop = 0;
		if(update == 2)
			gameSettings(&game, &draw);
		if(resizeFrame){
			screenResize(windowHandle.window.w, windowHandle.window.h);
			drawScale(&game, &draw);
		}
		
		// draw update
		if(update || firstFrame || resizeFrame){
			
			// draw update
			drawUpdate(&game, &draw);
			
			// game display
			screenClear();
			drawFuncs[game.isPaused](&draw);
			windowDisplay(&windowHandle);
			
			firstFrame--;
			resizeFrame = 0;
		}
		Sleep(framePeriod);
	}
	gameCleanup(&game);
	drawCleanup(&draw);
	
	return message;
}