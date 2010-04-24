#include "config.hh"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <cstdio>
#include <vector>

#include <boost/scoped_ptr.hpp>
#ifdef USE_THREADS
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#endif
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

static bool QUIT = false;

/// Keyboard input
void handle_keys(Players& players) {
	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		switch(event.type) {
		case SDL_QUIT:
			QUIT = true; return;
		case SDL_KEYDOWN: {
			int k = event.key.keysym.sym;
			if (k == SDLK_ESCAPE) { QUIT = true; return; }
			for (Players::iterator it = players.begin(); it != players.end(); ++it) {
				if (it->type != Actor::HUMAN) continue;
				if (k == it->KEY_LEFT) it->move(-1);
				else if (k == it->KEY_RIGHT) it->move(1);
				if (k == it->KEY_UP) it->jump();
				else if (k == it->KEY_DOWN) it->duck();
				if (k == it->KEY_ACTION) it->action();
			}
			break;
			}
		case SDL_KEYUP: {
			int k = event.key.keysym.sym;
			for (Players::iterator it = players.begin(); it != players.end(); ++it) {
				if (it->type != Actor::HUMAN) continue;
				if (k == it->KEY_UP || k == it->KEY_DOWN) it->end_jumping();
				if (k == it->KEY_LEFT || k == it->KEY_RIGHT) it->stop();
			}
			break;
		}
		} // end switch
	}
}

/// Double buffering
void flip() {
	SDL_GL_SwapBuffers();
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
}

/// OpenGL initialization
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

/// Thread functions

#ifdef USE_THREADS
void updateKeys(Players& players) {
	#ifndef WIN32 // SDL needs the keys to be in the main thread on Windows platform
	while (!QUIT) { handle_keys(players); }
	#endif
}
void updateWorld(World& world) { while (!QUIT) { world.update(); } }
void updateViewport(World& world) {
	while (!QUIT) {
		world.updateViewport();
		boost::this_thread::sleep(boost::posix_time::milliseconds(15));
	}
}
#endif

/// Game loop
bool main_loop(bool is_client, std::string host, int port) {
	if (is_client) srand(100);
	TextureMap tm = load_textures();
	World world(scrW, scrH, tm, !is_client);
	#ifdef USE_NETWORK
	Client client(&world);

	if (is_client) {
		client.connect(host, port);
		std::cout << "Connected to " << host << ":" << port << std::endl;
		std::cout << "Receiving initial data..." << std::endl;
		boost::this_thread::sleep(boost::posix_time::milliseconds(2000));
	} else {
	#else
	if (true) {
	#endif
		world.addActor(scrW-100, scrH/2, Actor::HUMAN, tm.find("tomato")->second);
		world.addActor(scrW-150, scrH/2, Actor::HUMAN, tm.find("tomato")->second);
		world.addActor(100, scrH/2, Actor::AI, tm.find("tomato")->second);
	}

	Players& players = world.getActors();
	parse_keys(players, "../keys.conf");

	// Launch threads
	#ifdef USE_THREADS
	boost::thread thread_input(updateKeys, boost::ref(players));
	boost::thread thread_physics(updateWorld, boost::ref(world));
	boost::thread thread_viewport(updateViewport, boost::ref(world));
	#endif

	// MAIN LOOP
	std::cout << "Game started." << std::endl;
	FPS fps;
	while (!QUIT) {
		fps.update();
		if ((SDL_GetTicks() % 500) == 0) fps.debugPrint();

		#if defined(WIN32) || !defined(USE_THREADS) // SDL needs the keys to be in the main thread on Windows platform
		handle_keys(players);
		#endif
		#ifndef USE_THREADS
		world.update();
		world.updateViewport();
		#else
		// Max ~ 100 fps is enough for graphics
		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
		#endif

		// Draw
		world.draw();
		flip();
	}
	#ifdef USE_NETWORK
	if (is_client) client.terminate();
	#endif
	#ifdef USE_THREADS
	thread_input.join();
	thread_physics.join();
	thread_viewport.join();
	#endif
	return false;
}

/// Server runs here
void server_loop(int port) {
#ifdef USE_NETWORK
	srand(100);
	TextureMap tm;
	World world(scrW, scrH, tm);
	//Players& players = world.getActors();
	Server server(&world, port);

	std::cout << "Server listening on port " << port << std::endl;

	// MAIN LOOP
	while (true) {
		// Update world
		world.update();
		// Compose game-state to send to clients
		std::string state = world.serialize();
		// Send game state to clients
		if (state != "") server.send_to_all(state);
	}
	server.terminate();
#endif
}

/// Program entry-point
int main(int argc, char** argv) {
	bool dedicated_server = false, client = false;
	std::string host("localhost");
	int port = DEFAULT_PORT;

	for (int i = 1; i < argc; i++) {
		std::string arg(argv[i]);
		if (arg == "--help" || std::string(argv[i]) == "-h") {
			std::cout << "Usage: " << argv[0] << " "
			  << "[--help | -h] [ [--server [port]] | [--client [host] [port]] ]"
			  << std::endl;
			return 0;
		}
		else if (arg == "--server") {
			dedicated_server = true;
			if (i < argc-1 && argv[i+1][0] != '-') { port = str2num<int>(std::string(argv[i+1])); ++i; }
		} else if (arg == "--client") {
			client = true;
			if (i < argc-1 && argv[i+1][0] != '-') {
				host = argv[i+1]; ++i;
				if (i < argc-1 && argv[i+1][0] != '-') { port = str2num<int>(std::string(argv[i+1])); ++i; }
			}
		}
	}

	srand(time(NULL)); // Randomize RNG
	#ifdef USE_NETWORK
	enet_initialize();
	#endif

	// TODO: Main menu

	#ifndef USE_NETWORK
	if (!dedicated_server && !client) {
	#endif
	if (!dedicated_server) {
		// SDL initialization stuff
		if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK) ==  -1) throw std::runtime_error("SDL_Init failed");
		SDL_WM_SetCaption(PACKAGE, PACKAGE);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_Surface* screen = NULL;
		screen = SDL_SetVideoMode(scrW, scrH, 32, SDL_OPENGL);
		if (!screen) throw std::runtime_error(std::string("SDL_SetVideoMode failed ") + SDL_GetError());
		SDL_EnableKeyRepeat(50, 50);

		setup_gl();
		main_loop(client, host, port);

		// TODO: SLD_Quit() hangs :(
		//SDL_Quit();
	} else server_loop(port);
	#ifndef USE_NETWORK
	} else {
		std::cout << "Networking support is disabled in this build." << std::endl;
	}
	#endif

	#ifdef USE_NETWORK
	enet_deinitialize();
	#endif
	return 0;
}
