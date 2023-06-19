#ifndef GAME
#define GAME

#include <stdio.h>
#include <time.h>

#define TITLE_WIDTH .7f
#define TITLE_HEIGHT .3f
#define BUTTON_TOTAL 3
#define BUTTON_WIDTH .5f
#define BUTTON_HEIGHT .1f
#define BUTTON_GAP .3f

#define GAME_WIDTH 30
#define GAME_HEIGHT 20
#define GAME_MINES 99

typedef struct Vector{
	float x;
	float y;
}Vector;

typedef struct Input{
	int toggleMenu;
	int select;
	int flag;
	Vector selectPos;
}Input;

typedef struct DrawData{
	
	// window
	Vector windowSize;
	
	// menu elements
	int menuState;
	Vector titlePos;
	Vector titleSize;
	Vector *buttonPos;
	Vector *buttonSize;
	
	// game board
	Vector gameSize; // proportion of screen (gameSize x, y <= windowSize x, y)
	int n;
	Vector *tile;
	Vector tileSize;
	int *value;
	
	// textures
	unsigned int texture;
	Vector texScale;
	Vector *titleUVLo;
	Vector *titleUVHi;
	Vector *buttonUVLo;
	Vector *buttonUVHi;
	Vector *tileUVLo;
	Vector *tileUVHi;
}DrawData;

typedef struct Game{
	
	// board settings
	int width;
	int height;
	int mines;
	int totalTiles;
	
	// current board
	int *tile;
	int *mask;
	int freeSpace;
	
	// menu
	Vector *button;
	Vector *buttonSize;
	
	// operational
	int *revealQueue; // used to reveal nearby tiles upon clicking an empty tile
	
	// game state
	int state; // -2: started, awaiting tile generation on click, -1: game lost, 1: game in progress (?), 2: game won
	int isPaused;
}Game;

// setup
int gameSetup(Game *game);
void gameClear(Game *game);
void gameStart(Game *game, int startTile);
void gameResize(Game *game);
void gameSettings(Game *game, DrawData *draw);
void inputSetup(Input *input);
int inputValue();

// update
int gameUpdate(Input *input, Game *game);
int menuUpdate(Input *input, Game *game);
void gamePause(Game *game);
void gameUnpause(Game *game);

// cursor
void gameCursorPos(Game *game, Input *input, DrawData *draw);
int gameTarget(Game *game, Input *input);
int menuTarget(Game *game, Input *input);

// game operations
void gameTileCheck(int target, int *queueSize, Game *game);
void gameTileReveal(int target, Game *game);
void gameShowBombs(Game *game);
void gameGetConnectedTiles(int target, Game *game, int *pos);

// drawing
int drawSetup(Game *game, DrawData *draw, unsigned int texture);
void drawGame(DrawData *draw);
void drawMenu(DrawData *draw);
void drawScale(Game *game, DrawData *draw);
void drawRetile(Game *game, DrawData *draw);
void drawUpdate(Game *game, DrawData *draw);

// cleanup
void gameCleanup(Game *game);
void drawCleanup(DrawData *draw);

// setup functions

int gameSetup(Game *game){
	
	// game
	game->width = GAME_WIDTH;
	game->height = GAME_HEIGHT;
	game->mines = GAME_MINES;
	game->totalTiles = game->width * game->height;
	game->tile = malloc(sizeof(int) * game->totalTiles);
	game->mask = malloc(sizeof(int) * game->totalTiles);
	game->revealQueue = malloc(sizeof(int) * game->totalTiles);
	if(game->tile == NULL || game->mask == NULL || game->revealQueue == NULL)
		return -1;
	gameClear(game);
	gamePause(game);
	
	// menu
	game->button = malloc(sizeof(Vector) * BUTTON_TOTAL);
	game->buttonSize = malloc(sizeof(Vector) * BUTTON_TOTAL);
	if(game->button == NULL || game->buttonSize == NULL)
		return -1;
	for(int b = 0; b < BUTTON_TOTAL; b++){
		float x = 0.f;
		float y = 0.f - (float)b * BUTTON_GAP;
		game->buttonSize[b].x = BUTTON_WIDTH;
		game->buttonSize[b].y = BUTTON_HEIGHT;
		game->button[b].x = (x + 1.f) / 2.f - game->buttonSize[b].x / 2.f;
		game->button[b].y = (y + 1.f) / 2.f - game->buttonSize[b].y / 2.f;
	}
	
	return 0;
}

void gameClear(Game *game){
	for(int i = 0; i < game->totalTiles; i++)
		game->mask[i] = 1;
	game->state = 0;
}

void gameStart(Game *game, int startTile){
	
	// reset board
	for(int t = 0; t < game->totalTiles; t++)
		game->tile[t] = 0;
	
	// place mines
	srand(time(NULL));
	int safezone[9];
	gameGetConnectedTiles(startTile, game, (int*)&safezone);
	safezone[8] = startTile;
	int bombs = 0;
	game->freeSpace = game->totalTiles - game->mines;
	
	// place mines near starting tile upon necessity
	int safeTiles = 0;
	for(int z = 0; z < 8; z++)
		safeTiles += safezone[z] != -1 ? 1 : 0;
	if(game->freeSpace - safeTiles <= 0){
		int safeAreaBombs = safeTiles - game->freeSpace + 1;
		int shuffle[8] = { 0, 1, 2, 3, 4, 5, 6, 7 }; // Fisher-Yates shuffle
		for(int i = 0; i < 7; i++){
			int j = i + rand() % (8 - i);
			int t = shuffle[i];
			shuffle[i] = shuffle[j];
			shuffle[j] = t;
		}
		for(int b = 0; bombs < safeAreaBombs; b++){
			if(safezone[shuffle[b]] >= 0){
				game->tile[safezone[shuffle[b]]] = -1;
				bombs++;
			}
		}
	}
	
	// place other mines away from starting tile
	while(bombs < game->mines){
		int bombPos = rand() % game->totalTiles;
		int setBomb = 1;
		for(int z = 0; z < 9; z++){
			if(bombPos == safezone[z] || game->tile[bombPos] == -1){
				setBomb = 0;
				break;
			}
		}
		game->tile[bombPos] = -setBomb;
		bombs += setBomb;
	}
	
	// count mines
	for(int t = 0; t < game->totalTiles; t++){
		if(game->tile[t] == -1) continue;
		
		int point[8];
		gameGetConnectedTiles(t, game, (int*)&point);
		
		int count = 0;
		for(int p = 0; p < 8; p++){
			if(point[p] != -1)
				count += (game->tile[point[p]] == -1 ? 1 : 0);
		}
		game->tile[t] = count;
	}
	
	// set play state
	game->state = 1;
}

void gameResize(Game *game){
	free(game->tile);
	free(game->mask);
	free(game->revealQueue);
	game->totalTiles = game->width * game->height;
	game->tile = malloc(sizeof(int) * game->totalTiles);
	game->mask = malloc(sizeof(int) * game->totalTiles);
	game->revealQueue = malloc(sizeof(int) * game->totalTiles);
}

void gameSettings(Game *game, DrawData *draw){
	
	// set game options
	int settingsAccepted = 0;
	while(!settingsAccepted){
		settingsAccepted = 1;
		printf("Set GAME_WIDTH: ");
		game->width = inputValue();
		printf("Set GAME_HEIGHT: ");
		game->height = inputValue();
		printf("Set GAME_MINES: ");
		game->mines = inputValue();
		if(game->width * game->height <= game->mines){
			printf("Settings not accepted: mines overfill field\n");
			settingsAccepted = 0;
		}
	}
	
	// resize
	gameResize(game);
	drawRetile(game, draw);
	
	// reset game
	gameClear(game);
	
	printf("Game Set\n");
}

void inputSetup(Input *input){
	input->toggleMenu = 0;
	input->select = 0;
	input->flag = 0;
	input->selectPos.x = -1;
	input->selectPos.y = -1;
}

int inputValue(){
	char input[256];
	int num=0;
	
	gets(input);
	
	for(int i=0;i<256;i++){
		if(input[i]=='\0')
			break;
		num*=10;
		num+=input[i]-48;
		if(i>4)
			printf("hit max integer\n");
	}
	
	return num;
}

// update functions

int gameUpdate(Input *input, Game *game){
	int targetTile = gameTarget(game, input);
	
	// select tile
	if(input->select){
		if(targetTile >= 0 && targetTile < game->totalTiles){
			
			if(game->state == 0) // start game
				gameStart(game, targetTile);
			
			if(game->mask[targetTile] == 1){ // reveal tile
				gameTileReveal(targetTile, game);
				
				if(game->tile[targetTile] == -1){ // game lost
					game->state = 2;
					gameShowBombs(game);
					gamePause(game);
				}
				
				else if(game->freeSpace <= 0){ // game won
					game->state = 3;
					gamePause(game);
				}
			}
		}
		input->select = 0;
		return 1;
	}
	
	// flag bomb on tile
	if(input->flag){
		if(targetTile >= 0){
			if(game->mask[targetTile] == 1)
				game->mask[targetTile] = 2;
			else if(game->mask[targetTile] == 2)
				game->mask[targetTile] = 1;
		}
		input->flag = 0;
		return 1;
	}
	
	// switch to menu
	if(input->toggleMenu){
		gamePause(game);
		input->toggleMenu = 0;
		return 1;
	}
	
	return 0;
}

int menuUpdate(Input *input, Game *game){
	
	// select button
	if(input->select){
		int target = menuTarget(game, input);
		int updateType = 1;
		switch(target){
			case 1: // Continue/Reset
				if(game->state == 2 || game->state == 3) // new game
					gameClear(game);
				gameUnpause(game);
				break;
			case 2: // Settings
				updateType = 2;
				break;
			case 3: // Quit
				updateType = -1;
				break;
		}
		input->select = 0;
		return updateType;
	}
	
	// switch to game
	if(input->toggleMenu){
		gameUnpause(game);
		input->toggleMenu = 0;
		return 1;
	}
	
	return 0;
}

void gamePause(Game *game){
	game->isPaused = 1;
}

void gameUnpause(Game *game){
	game->isPaused = 0;
}

// cursor functions

void gameCursorPos(Game *game, Input *input, DrawData *draw){
	input->selectPos.x = (input->selectPos.x / draw->gameSize.x + 1.f) / 2.f;
	input->selectPos.y = (input->selectPos.y / draw->gameSize.y + 1.f) / 2.f;
}

int gameTarget(Game *game, Input *input){
	int x = (int)(input->selectPos.x * game->width);
	int y = (int)(input->selectPos.y * game->height);
	if(x >= 0 && x < game->width && y >= 0 && y < game->height)
		return x + (y * game->width);
	return -1;
}

int menuTarget(Game *game, Input *input){
	for(int b = 0; b < 3; b++){
		if(
				input->selectPos.x >= game->button[b].x && 
				input->selectPos.x < game->button[b].x + game->buttonSize[b].x && 
				input->selectPos.y >= game->button[b].y && 
				input->selectPos.y < game->button[b].y + game->buttonSize[b].y)
			return b + 1;
	}
	return -1;
}

// game operation functions

void gameTileCheck(int target, int *queueSize, Game *game){
	if(game->mask[target] == 1){
		game->revealQueue[*queueSize] = target;
		*queueSize += 1;
		game->mask[target] = 0;
	}
}

void gameTileReveal(int target, Game *game){
	game->mask[target] = 0;
	int *reveal = game->revealQueue;
	reveal[0] = target;
	int queueSize = 1;
	
	int spaceFreed;
	for(spaceFreed = 0; spaceFreed < queueSize; spaceFreed++){
		if(game->tile[reveal[spaceFreed]] == 0){
			int pos[8];
			gameGetConnectedTiles(reveal[spaceFreed], game, (int*)&pos);
			for(int i = 0; i < 8; i++){
				if(pos[i] >= 0)
					gameTileCheck(pos[i], &queueSize, game);
			}
		}
	}
	game->freeSpace -= spaceFreed;
}

void gameShowBombs(Game *game){
	int i;
	for(i = 0; i < game->totalTiles; i++)
		if(game->mask[i] == 1 && game->tile[i] == -1)
			game->mask[i] = 0;
}

void gameGetConnectedTiles(int target, Game *game, int *pos){
	pos[0] = target - game->width - 1;
	pos[1] = target - game->width;
	pos[2] = target - game->width + 1;
	pos[3] = target - 1;
	pos[4] = target + 1;
	pos[5] = target + game->width - 1;
	pos[6] = target + game->width;
	pos[7] = target + game->width + 1;
	if(target < game->width)
		pos[0] = pos[1] = pos[2] = -1;
	if(target >= game->width * game->height - game->width)
		pos[5] = pos[6] = pos[7] = -1;
	if(target % game->width == 0)
		pos[0] = pos[3] = pos[5] = -1;
	if(target % game->width == game->width - 1)
		pos[2] = pos[4] = pos[7] = -1;
}

// drawing functions

int drawSetup(Game *game, DrawData *draw, unsigned int texture){
	
	// data allocation
	draw->n = game->totalTiles;
	draw->tile = malloc(sizeof(Vector) * draw->n);
	draw->value = malloc(sizeof(float) * draw->n);
	draw->buttonPos = malloc(sizeof(Vector) * BUTTON_TOTAL);
	draw->buttonSize = malloc(sizeof(Vector) * BUTTON_TOTAL);
	draw->titleUVLo = malloc(sizeof(Vector) * 4);
	draw->titleUVHi = malloc(sizeof(Vector) * 4);
	draw->buttonUVLo = malloc(sizeof(Vector) * BUTTON_TOTAL);
	draw->buttonUVHi = malloc(sizeof(Vector) * BUTTON_TOTAL);
	draw->tileUVLo = malloc(sizeof(Vector) * 12);
	draw->tileUVHi = malloc(sizeof(Vector) * 12);
	if(
			draw->tile == NULL || draw->value == NULL || draw->buttonPos == NULL || draw->buttonSize == NULL || 
			draw->titleUVLo == NULL || draw->titleUVHi == NULL || draw->buttonUVLo == NULL || draw->buttonUVHi == NULL || 
			draw->tileUVLo == NULL || draw->tileUVHi == NULL)
		return -1;
	
	// data initialisation
	draw->titlePos.x = 0.f;
	draw->titlePos.y = .65f;
	draw->titleSize.x = TITLE_WIDTH;
	draw->titleSize.y = TITLE_HEIGHT;
	for(int b = 0; b < BUTTON_TOTAL; b++){
		draw->buttonPos[b].x = 0.f;
		draw->buttonPos[b].y = 0.f - (float)b * BUTTON_GAP;
		draw->buttonSize[b].x = BUTTON_WIDTH;
		draw->buttonSize[b].y = BUTTON_HEIGHT;
	}
	drawUpdate(game, draw);
	
	// textures and half-pixel corrected uv coordinates
	draw->texture = texture;
	draw->texScale.x = 1.f / TEXTURE_WIDTH;
	draw->texScale.y = 1.f / TEXTURE_HEIGHT;
	for(int t = 0; t < 4; t++){ // menu title
		draw->titleUVLo[t].x = draw->texScale.x * .5f;
		draw->titleUVHi[t].x = draw->texScale.x * 55 + draw->titleUVLo[t].x;
		draw->titleUVLo[t].y = draw->texScale.y * (8.5f + t * 12);
		draw->titleUVHi[t].y = draw->texScale.y * 11 + draw->titleUVLo[t].y;
	}
	for(int b = 0; b < BUTTON_TOTAL; b++){ // menu buttons
		draw->buttonUVLo[b].x = draw->texScale.x * .5f;
		draw->buttonUVHi[b].x = draw->texScale.x * 31 + draw->buttonUVLo[b].x;
		draw->buttonUVLo[b].y = draw->texScale.y * (56.5f + b * 8);
		draw->buttonUVHi[b].y = draw->texScale.y * 7 + draw->buttonUVLo[b].y;
	}
	for(int t = 0; t < 12; t++){ // game tiles
		draw->tileUVLo[t].x = draw->texScale.x * (.5f + t * 8);
		draw->tileUVHi[t].x = draw->texScale.x * 7 + draw->tileUVLo[t].x;
		draw->tileUVLo[t].y = draw->texScale.y * .5f;
		draw->tileUVHi[t].y = draw->texScale.y * 7 + draw->tileUVLo[t].y;
	}
	
	return 0;
}

void drawGame(DrawData *draw){
	for(int t = 0; t < draw->n; t++){
		drawTexture(
			draw->tile[t].x, draw->tile[t].y, draw->tileSize.x, draw->tileSize.y, 
			draw->texture, draw->tileUVLo[draw->value[t]].x, draw->tileUVHi[draw->value[t]].x, draw->tileUVHi[draw->value[t]].y, draw->tileUVLo[draw->value[t]].y);
	}
}

void drawMenu(DrawData *draw){
	
	// board background
	drawGame(draw);
	
	// title
	drawTexture(
		draw->titlePos.x * draw->gameSize.x, draw->titlePos.y * draw->gameSize.y, 
		draw->titleSize.x * draw->gameSize.x, draw->titleSize.y * draw->gameSize.y, 
		draw->texture, draw->titleUVLo[draw->menuState].x, draw->titleUVHi[draw->menuState].x, draw->titleUVHi[draw->menuState].y, draw->titleUVLo[draw->menuState].y);
	
	// buttons
	for(int b = 0; b < BUTTON_TOTAL; b++){
		drawTexture(
			draw->buttonPos[b].x * draw->gameSize.x, draw->buttonPos[b].y * draw->gameSize.y, 
			draw->buttonSize[b].x * draw->gameSize.x, draw->buttonSize[b].y * draw->gameSize.y,  
			draw->texture, draw->buttonUVLo[b].x, draw->buttonUVHi[b].x, draw->buttonUVHi[b].y, draw->buttonUVLo[b].y);
	}
}

void drawScale(Game *game, DrawData *draw){
	
	// window-relative game size (x, y <= 1 proportions of window region)
	float gameRatio = (float)game->width / (float)game->height;
	draw->gameSize.x = gameRatio / (draw->windowSize.x / draw->windowSize.y);
	draw->gameSize.y = 1.f;
	if(draw->gameSize.x > 1.f){
		draw->gameSize.y /= draw->gameSize.x;
		draw->gameSize.x = 1.f;
	}
	
	// game-relative tile size & positions
	draw->tileSize.x = draw->gameSize.x / game->width;
	draw->tileSize.y = draw->gameSize.y / game->height;
	int index = 0;
	for(int j = 0; j < game->height; j++){
		for(int i = 0; i < game->width; i++){
			draw->tile[index].x = (float)(i + .5f - .5f * game->width) * 2.f * draw->tileSize.x;
			draw->tile[index].y = (float)(j + .5f - .5f * game->height) * 2.f * draw->tileSize.y;
			index++;
		}
	}
}

void drawRetile(Game *game, DrawData *draw){
	free(draw->tile);
	free(draw->value);
	draw->n = game->totalTiles;
	draw->tile = malloc(sizeof(Vector) * draw->n);
	draw->value = malloc(sizeof(float) * draw->n);
	drawScale(game, draw);
}

void drawUpdate(Game *game, DrawData *draw){
	for(int i = 0; i < game->totalTiles; i++)
		draw->value[i] = (game->mask[i] == 0 ? (game->tile[i] == -1 ? 9 : game->tile[i]) : 9 + game->mask[i]);
	draw->menuState = game->state;
}

// cleanup functions

void gameCleanup(Game *game){
	free(game->tile);
	free(game->mask);
	free(game->revealQueue);
	free(game->button);
	free(game->buttonSize);
}

void drawCleanup(DrawData *draw){
	free(draw->buttonPos);
	free(draw->buttonSize);
	free(draw->tile);
	free(draw->value);
	free(draw->titleUVLo);
	free(draw->titleUVHi);
	free(draw->buttonUVLo);
	free(draw->buttonUVHi);
	free(draw->tileUVLo);
	free(draw->tileUVHi);
}

#endif