#include "Engine/GLApplication.h"

int main() {
	Game& game = Game::instance();

	if (!game.init()) {
		return -1;
	}
	
	game.run();

	//Game::deleteInstace();

	return 0;
}