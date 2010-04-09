#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <cstdio>
#include <vector>

#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL.h>

#include "util.hh"
#include "player.hh"
#include "world.hh"
#include "network.hh"
#include "keys.hh"
#include "texture.hh"

#define scrW 800
#define scrH 600


bool handle_keys(Players& players) {
	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		switch(event.type) {
		case SDL_QUIT:
			return false;
		case SDL_KEYDOWN: {
			int k = event.key.keysym.sym;
			if (k == SDLK_ESCAPE) return false;
			for (Players::iterator it = players.begin(); it != players.end(); ++it) {
				if (k == it->KEY_LEFT) it->move(-1);
				else if (k == it->KEY_RIGHT) it->move(1);
				if (k == it->KEY_UP && it->can_jump()) it->jump();
				else if (k == it->KEY_DOWN) it->duck();
				if (k == it->KEY_ACTION) it->action();
			}
			break;
			}
		case SDL_KEYUP: {
			int k = event.key.keysym.sym;
			for (Players::iterator it = players.begin(); it != players.end(); ++it) {
				if (k == it->KEY_UP) it->jumping = 0;
				//else if (k == it->KEY_DOWN) it->duck();
			}
			break;
		}
		} // end switch
	}
	return true;
}

void flip() {
	SDL_GL_SwapBuffers();
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
}

void setup_gl() {
	glViewport(0, 0, scrW, scrH);
	glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0, scrW, scrH, 0);
	glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	glShadeModel(GL_SMOOTH);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glColor4ub(255, 255, 255, 255);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
}


bool main_loop() {
	TextureMap tm = load_textures();
	World world(scrW, scrH, tm);
	world.addActor(scrW-100, scrH/2, tm.find("tomato")->second);
	world.addActor(100, scrH/2, tm.find("tomato")->second);
	Players& players = world.getActors();
	parse_keys(players, "../keys.conf");

	// MAIN LOOP
	while (handle_keys(players)) {
		world.update();
		world.draw();
		flip();
	}

	return false;
}


int main(int argc, char** argv) {
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK) ==  -1) throw std::runtime_error("SDL_Init failed");
	//SDL_WM_SetCaption(PACKAGE " " VERSION, PACKAGE);

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_Surface* screen = 0;
	screen = SDL_SetVideoMode(800, 600, 32, SDL_OPENGL);
	if (!screen) throw std::runtime_error(std::string("SDL_SetVideoMode failed ") + SDL_GetError());
	SDL_EnableKeyRepeat(80, 80);

	setup_gl();
	srand(time(NULL)); // Randomize RNG

	main_loop();

	// TODO: SDL_Quit creates a crash if Texture_2D is enabled in player.draw()
	//SDL_Quit();
	return 0;
}
