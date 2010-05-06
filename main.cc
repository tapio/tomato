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

#include "GLFT_Font.hh"
#include "util.hh"
#include "settings.hh"
#include "filesystem.hh"
#include "player.hh"
#include "world.hh"
#include "network.hh"
#include "keys.hh"
#include "texture.hh"
#include "gamemode.hh"

#define WW 25.0
#define WH (WW*scrH/scrW)

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
				if (it->type != Actor::HUMAN || it->is_dead()) continue;
				if (k == it->KEY_LEFT) it->key_left = true;
				else if (k == it->KEY_RIGHT) it->key_right = true;
				if (k == it->KEY_UP) it->key_up = true;
				else if (k == it->KEY_DOWN) it->key_down = true;
				// Action button doesn't require state tracking
				if (k == it->KEY_ACTION) it->action();
			}
			break;
			}
		case SDL_KEYUP: {
			int k = event.key.keysym.sym;
			for (Players::iterator it = players.begin(); it != players.end(); ++it) {
				if (it->type != Actor::HUMAN || it->is_dead()) continue;
				if (k == it->KEY_UP) { if (it->key_up) it->end_jumping(); it->key_up = false; }
				if (k == it->KEY_DOWN) { if (it->key_down) it->end_jumping(); it->key_down = false; }
				if (k == it->KEY_LEFT) { if (it->key_left) it->stop(); it->key_left = false; }
				if (k == it->KEY_RIGHT) { if (it->key_right) it->stop(); it->key_right = false; }
			}
			break;
		}
		} // end switch
	}
	for (Players::iterator it = players.begin(); it != players.end(); ++it) {
		if (it->type != Actor::HUMAN || it->is_dead()) continue;
		if (it->key_left) it->move(-1);
		else if (it->key_right) it->move(1);
		if (it->key_up) it->jump();
		else if (it->key_down) it->duck();
	}
}

/// Double buffering
void flip() {
	SDL_GL_SwapBuffers();
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	glColor4f(1,1,1,1);
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
	glEnable(GL_LINE_SMOOTH);
}

/// Thread functions

#ifdef USE_THREADS
void updateKeys(Players& players) {
	#ifndef WIN32 // SDL needs the keys to be in the main thread on Windows platform
	while (!QUIT) {
		handle_keys(players);
		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	}
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
bool main_loop(GameMode gm, int num_players_local, int num_players_ai, bool is_client, std::string host, int port) {
	TextureMap tm = load_textures();
	World world(WW, WH, tm, gm, !is_client);
	Players& players = world.getActors();

	// Load font
	GLFT_Font f;
	f.open(getFilePath("fonts/FreeSerifBold.ttf"), 16);

	// Draw title
	const int titlew = scrW/2, titleh = titlew/2;
	drawImage(tm.find("title")->second, scrW/2 - titlew/2, scrH/2 - titleh/2, titlew, titleh);
	flip();
	double titletime = GetSecs() + 0.75;

	#ifdef USE_NETWORK
	Client client(&world);

	if (is_client) {
		client.connect(host, port);
		std::cout << "Connected to " << host << ":" << port << std::endl;
		std::cout << "Waiting for players..." << std::endl;
		while (true) {
			boost::this_thread::sleep(boost::posix_time::milliseconds(500));
			if (players.size() >= 2) break;
		}
	} else {
	#else
	if (true) {
	#endif
		for (int i = 0; i < num_players_local + num_players_ai; ++i) {
			b2Vec2 pos = world.randomSpawnLocked();
			world.addActor(pos.x, pos.y, i >= num_players_local ? Actor::AI : Actor::HUMAN, (i % 4) + 1);
		}
	}

	parse_keys(players, getFilePath("data/keys.conf"));

	while (titletime > GetSecs()); // Ensure title visibility

	// Launch threads
	#ifdef USE_THREADS
	boost::thread thread_input(updateKeys, boost::ref(players));
	boost::thread thread_physics(updateWorld, boost::ref(world));
	boost::thread thread_viewport;
	if (config_zoom) thread_viewport = boost::thread(updateViewport, boost::ref(world));
	#endif

	// MAIN LOOP
	std::cout << "Game started." << std::endl;
	FPS fps;
	while (!QUIT) {
		fps.update();
		if ((int(GetSecs()*1000) % 500) == 0) fps.debugPrint();

		#if defined(WIN32) || !defined(USE_THREADS) // SDL needs the keys to be in the main thread on Windows platform
		handle_keys(players);
		#endif
		#if !defined(USE_THREADS)
		world.update();
		if (config_zoom) world.updateViewport();
		#elif !defined(WIN32)
		// Max ~ 100 fps is enough for graphics
		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
		#endif

		// Render world
		world.draw();

		// Draw UI
		glEnable(GL_TEXTURE_2D);
		std::ostringstream oss; int i = 1;
		for (Players::const_iterator it = players.begin(); it != players.end(); ++it, ++i)
			oss << "Player " << i << ": " << it->points.round_score << "   ";
		glColor4f(1.0f,0.0f,0.0f,0.75f);
		f.beginDraw(10, 10) << oss.str() << f.endDraw();

		// Flip
		flip();
	}
	#ifdef USE_NETWORK
	if (is_client) client.terminate();
	#endif
	#ifdef USE_THREADS
	thread_input.join();
	thread_physics.join();
	if (config_zoom) thread_viewport.join();
	#endif
	return false;
}

/// Server runs here
void server_loop(GameMode gm, int port) {
#ifdef USE_NETWORK
	TextureMap tm;
	World world(WW, WH, tm, gm);
	Players& players = world.getActors();
	Server server(&world, port);

	std::cout << "Server listening on port " << port << std::endl;

	// Wait for players before starting simulation
	while (true) {
		boost::this_thread::sleep(boost::posix_time::milliseconds(500));
		if (players.size() >= 2) break;
	}

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

/// Parse the cmd line argument's following value to variable
template<typename T> bool parseVal(T& var, int& i, int argc, char** argv) {
	if (i < argc-1 && argv[i+1][0] != '-') {
		var = str2num<T>(std::string(argv[i+1]));
		++i;
		return true;
	}
	return false;
}
template<> bool parseVal(std::string& var, int& i, int argc, char** argv) {
	if (i < argc-1 && argv[i+1][0] != '-') {
		var = std::string(argv[i+1]);
		++i;
		return true;
	}
	return false;
}

/// Program entry-point
int main(int argc, char** argv) {
	bool dedicated_server = false, client = false;

	readConfig();

	std::string gamemode(config_default_gamemode);
	std::string host(config_default_host);
	int port = config_default_port;
	int num_players_local = 2;
	int num_players_ai = 0;

	for (int i = 1; i < argc; i++) {
		std::string arg(argv[i]);
		if (arg == "--help" || std::string(argv[i]) == "-h") {
			std::cout << "Usage: " << argv[0] << " "
			  << "[--help | -h] [--players NUM] [--ai NUM] [ [--server [PORT]] | [--client [HOST] [PORT]] ]"
			  << std::endl;
			return 0;
		}
		else if (arg == "--server") {
			dedicated_server = true;
			parseVal(port, i, argc, argv);
		} else if (arg == "--client") {
			client = true;
			if (parseVal(host, i, argc, argv))
				parseVal(port, i, argc, argv);
		} else if (arg == "--players") parseVal(num_players_local, i, argc, argv);
		else if (arg == "--ai") parseVal(num_players_ai, i, argc, argv);
		else if (arg == "--gamemode") parseVal(gamemode, i, argc, argv);
		else {
			std:: cout << "Unrecognized option '" << arg << "'. Use --help for usage info." << std::endl;
			exit(EXIT_FAILURE);
		}
	}

	srand(time(NULL)); // Randomize RNG
	#ifdef USE_NETWORK
	enet_initialize();
	#endif

	// TODO: Main menu

	try {
		if (gamemode.find(".gamemode") == std::string::npos) gamemode += ".gamemode";
		GameMode gm(getFilePath("data/" + gamemode));

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
			screen = SDL_SetVideoMode(scrW, scrH, 32, SDL_OPENGL | (config_fullscreen ? SDL_FULLSCREEN : 0));
			if (!screen) throw std::runtime_error(std::string("SDL_SetVideoMode failed ") + SDL_GetError());

			setup_gl();
			main_loop(gm, num_players_local, num_players_ai, client, host, port);

			// FIXME: SLD_Quit() hangs :(
			//SDL_Quit();
		} else server_loop(gm, port);
		#ifndef USE_NETWORK
		else {
			std::cout << "Networking support is disabled in this build." << std::endl;
		}
		}
		#endif
	} catch (std::exception& e) {
		// TODO: Nicer output
		std::cout << "-!- FATAL ERROR: " << e.what() << std::endl;
	}
	#ifdef USE_NETWORK
	enet_deinitialize();
	#endif
	#ifdef WIN32
	// FIXME: Quirk to not hang on exit on Windows
	_exit(0);
	#endif
	return 0;
}
