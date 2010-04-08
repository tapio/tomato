#include <iostream>
#include <string>
#include <stdexcept>
#include <cstdio>
#include <vector>

#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL.h>

#include "player.hh"
#include "world.hh"
#include "network.hh"

#define scrW 800
#define scrH 600

bool handle_keys() {
	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		switch(event.type) {
		case SDL_KEYDOWN:
			return false;
		case SDL_QUIT:
			return false;
		}
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
	glEnable(GL_TEXTURE_2D);
}


bool main_loop() {
	World world(scrW, scrH);
	typedef std::vector<Player> Players;
	Players players;
	players.push_back(Player(world, 0, 0));
	players.push_back(Player(world, 100, 100));

	// MAIN LOOP
	while (handle_keys()) {
		world.draw();
		flip();
	}
	return false;
}


int main(int argc, char** argv) {
	std::atexit(SDL_Quit);
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK) ==  -1) throw std::runtime_error("SDL_Init failed");
	//SDL_WM_SetCaption(PACKAGE " " VERSION, PACKAGE);

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_Surface* screen = 0;
	screen = SDL_SetVideoMode(800, 600, 32, SDL_OPENGL);
	if (!screen) throw std::runtime_error(std::string("SDL_SetVideoMode failed ") + SDL_GetError());

	setup_gl();

	main_loop();

	return 0;
}
