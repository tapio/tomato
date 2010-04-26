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
#include <SFML/Window.hpp>

#include "util.hh"
#include "filesystem.hh"
#include "player.hh"
#include "world.hh"
#include "network.hh"
#include "keys.hh"
#include "texture.hh"

#define scrW 800
#define scrH 600

static bool QUIT = false;

/// Keyboard input
void handle_keys(sf::Window& app, Players& players) {
	sf::Event event;
	int counter = 0; // Hack to prevent bogus joystick events stealing the show
	while(counter < 10 && app.GetEvent(event)) {
		counter++;
		switch(event.Type) {
		case sf::Event::Closed:
			QUIT = true; return;
		case sf::Event::KeyPressed: {
			int k = event.Key.Code;
			if (k == sf::Key::Escape) { QUIT = true; return; }
			for (Players::iterator it = players.begin(); it != players.end(); ++it) {
				if (it->type != Actor::HUMAN) continue;
				if (k == it->KEY_LEFT) it->key_left = true;
				else if (k == it->KEY_RIGHT) it->key_right = true;
				if (k == it->KEY_UP) it->key_up = true;
				else if (k == it->KEY_DOWN) it->key_down = true;
				// Action button doesn't require state tracking
				if (k == it->KEY_ACTION) it->action();
			}
			break;
			}
		case sf::Event::KeyReleased: {
			int k = event.Key.Code;
			for (Players::iterator it = players.begin(); it != players.end(); ++it) {
				if (it->type != Actor::HUMAN) continue;
				if (k == it->KEY_UP) { if (it->key_up) it->end_jumping(); it->key_up = false; }
				if (k == it->KEY_DOWN) { if (it->key_down) it->end_jumping(); it->key_down = false; }
				if (k == it->KEY_LEFT) { if (it->key_left) it->stop(); it->key_left = false; }
				if (k == it->KEY_RIGHT) { if (it->key_right) it->stop(); it->key_right = false; }
			}
			break;
		}
		default: break;
		} // end switch
	}
	// Do effects
	for (Players::iterator it = players.begin(); it != players.end(); ++it) {
		if (it->type != Actor::HUMAN) continue;
		if (it->key_left) it->move(-1);
		else if (it->key_right) it->move(1);
		if (it->key_up) it->jump();
		else if (it->key_down) it->duck();
	}
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

struct InputWrapper {
	InputWrapper(sf::Window& a, Players& p): app(a), players(p) { }
	sf::Window& app;
	Players& players;
};

#ifdef USE_THREADS
void updateKeys(InputWrapper& iw) {
	while (!QUIT) { handle_keys(iw.app, iw.players); }
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
	// Window initialization stuff
	sf::Window App(sf::VideoMode(scrW, scrH, 32), PACKAGE);
	setup_gl();

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
	parse_keys(players, "keys.conf");

	// Launch threads
	#ifdef USE_THREADS
	//boost::thread thread_input(updateKeys, InputWrapper(App, players));
	boost::thread thread_physics(updateWorld, boost::ref(world));
	boost::thread thread_viewport(updateViewport, boost::ref(world));
	#endif

	// MAIN LOOP
	std::cout << "Game started." << std::endl;
	FPS fps;
	while (!QUIT && App.IsOpened()) {
		fps.update();
		if ((int(GetSecs()*1000) % 500) == 0) fps.debugPrint();

		handle_keys(App, players);

		#ifndef USE_THREADS
		world.update();
		world.updateViewport();
		#else
		// Max ~ 100 fps is enough for graphics
		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
		#endif

		// Draw
		world.draw();
		App.Display();
		glClear(GL_COLOR_BUFFER_BIT);
	}
	QUIT = true;
	#ifdef USE_NETWORK
	if (is_client) client.terminate();
	#endif
	#ifdef USE_THREADS
	//thread_input.join();
	thread_physics.join();
	thread_viewport.join();
	#endif
	return false;
}

/// Server runs here
void server_loop(int port) {
#ifdef USE_NETWORK
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
		main_loop(client, host, port);
	} else server_loop(port);
	#ifndef USE_NETWORK
	} else {
		std::cout << "Networking support is disabled in this build." << std::endl;
	}
	#endif
	#ifdef USE_NETWORK
	enet_deinitialize();
	#endif
	#ifdef WIN32
	// FIXME: Quirk to not hang on exit on Windows
	_exit(0);
	#endif
	return 0;
}
