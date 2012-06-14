// Copyright © 2012 the Minima Authors under the MIT license. See AUTHORS for the list of authors.
#include "ui.hpp"
#include "game.hpp"
#include "io.hpp"
#include "screens.hpp"
#include "world.hpp"
#include <vector>
#include <SDL_main.h>

// drawHeights, when set to true makes the world draw the
// heigth of each tile on it.
extern bool drawHeights;

static void parseArgs(int, char*[]);

int main(int argc, char *argv[]) try{
	parseArgs(argc, argv);

	Ui win (ScreenDims.x, ScreenDims.y, "Minima");

	ScreenStack stk(win, NewTitleScreen());
	stk.Run();

	return 0;

}catch (const std::exception &f) {
	printf(cerr(), "Uncaught exception: \"%v\"\n", f.what());
	return 1;
}

static void parseArgs(int argc, char *argv[]) {
	vector<string> args (argv+1, argv+argc);

	for(auto &arg : args){
		if (arg == "-heights")
			drawHeights = true;
	}
}

