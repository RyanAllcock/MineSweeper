~~~ Mine Sweeper ~~~

~ How To Use:
	- Compile using attached batch script "compile.bat", usingthe MinGW  Windows and OpenGL standard libraries
	- Run "minesweeper.exe" to play

~ In This Project:
	- C implementation of Minesweeper, using the Win32 API and OpenGL intermediate-mode for window-handling and screen-drawing
	- All files except for "stb_image.h" were produced independently for this project

~ Features:
	- Fully-textured environment
	- Resizable window, with dynamic graphics scaling to fit the window without stretching
	- Options for the board's grid size (subject to platform-dependent size limitations)

~ Internal Structures:
	- MVC code structure, between "game.h", "window.h" and "graphics.h"
	- Function pointers, used for stateful game updates and screen drawing
	- Mouse cursor and keyboard input
	- Callback function for window input handling

~ Development Ideas:
	- [new modules]
		- leaderboard (eventually stored in a file)
		- timer
	- [split game and menu-game interactions by file]
	- [peripheral input maps]
	- [more compact way of storing game] may not be viable; storing mine fronts but not storing fully-empty space somehow?
	- [better bomb placing strategy] avoid placing bombs on the same spot twice, by perhaps using a diminishing-eligible-mine-region queue
	- [grid data/drawing structure] also maybe a menu data/drawing structure

~ Problem Log:
	- [general]
		- [add comments]
		- [clean up main]
	- [ensure function arguments are variable references instead of structs where viable]
	- [define game states as constants]
	- [DrawData struct] possibly find better name
		- [streamline DrawData variables, usage in functions, and utilisation between window operations and game operations]
		- [avoid sending game data to DrawData struct] only send colour/id data to struct, leave nothing to switch statements for each tile's mask & value etc. (only "value"s no "mask"s should be seen by draw functions)
		- [replace DrawData tile vectors with offsets + strides + width/height (keep scales in ofc) ?]
	- [limit unnecessary draws]
		- [remove black background on first menu frame] figure out why it happens, then streamline initial drawing
		- [only draw when the game/window has just been updated] avoid unnecessary draws, after new settings are input or at startup
	- [menu data improvements]
		- [menu data structure] for advanced menu button & visual-element placement/management
		- [connect button data for drawing & input] without the use of constants, for more interesting menu designs later
	- [in-screen game options input method]
		- can't click start button when new game options are chosen (happens with exit button too? maybe options button as well then)
		- [non-window-freezing settings prompt] replace settings input with in-screen prompt (could do textures first, then add text boxes for inputting settings)
		- [fully remove need for local #include<stdio.h> in game.h file]
	- [window management]
		- [dynamic-display screen during window resize/move]
		- [check if message peeking/dispatching in windowUpdate is received before or after the callback] if received before then the handle struct is a frame behind in windowUpdate...
		- [better framerate handling strategy]
		- [revisit callback utilisation] some operations might be best placed in windowUpdate/callback that are currently inside the opposite one
	- [textures]
		- [move graphics colours etc. to lookup tables] ready for texture uv coordinate translations input
		- [compensate for texture-embedded border in tile targetting code]
		- [store uv coordinates & texture references for each element together in e.g. draw data] texture size is currently hard-coded in draw functions, for uv coordinate computing
		- [draw some texture elements by true size proportions] for e.g. menu title which doesn't need input boundaries
		- [somehow read in texture uv coordinates] remove & abstract-away any hard-coded drawing data, including menu elements positions, any texture ids, and any uv coordinate data (texture atlas text-.atlas file, possibly with embedded element indices-positions?)
