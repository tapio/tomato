#pragma once

#include "config.hh"
#ifdef USE_THREADS
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#endif

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <GL/gl.h>
#include <Box2D.h>

#include "texture.hh"
#include "powerups.hh"
#include "util.hh"
#include "player.hh"
#include "worldelements.hh"
#include "gamemode.hh"

#define GRAVITY 2.5f

class Client;

class World {
  public:
	World(int width, int height, TextureMap& tm, GameMode gm, bool master = true);

	Actor* shoot(const Actor& shooter);
	void kill(Actor* target, Actor* killer = NULL);
	bool safe2spawn(float x, float y) const;
	b2Vec2 randomSpawn() const;
	b2Vec2 randomSpawnLocked() const;

	void addMine(float x, float y);
	void addActor(float x, float y, Actor::Type type, int character = 1, Client* client = NULL);
	void addActorBody(float x, float y, Actor* actor);
	bool addPlatform(float x, float y, float w, bool force = false);
	void addLadder(float x, float y, float h);
	void addCrate(float x, float y);
	void addBridge(unsigned leftAnchorID, unsigned rightAnchorID);
	void addPowerup(float x, float y, Powerup::Type type);

	void generateBorders();
	void generateLevel();
	void newRound();

	std::string serialize(bool skip_static = true) const;
	void update();
	void update(std::string data, Client* client = NULL);
	void updateViewport();
	void draw() const;

	b2World& getWorld() { return world; }
	Actors& getActors() { return actors; }

  private:
	#ifdef USE_THREADS
	mutable boost::mutex mutex;
	#endif
	bool is_master;
	b2World world;
	float w;
	float h;
	float SCALE;
	b2Vec2 view_topleft;
	b2Vec2 view_bottomright;
	float tilesize;
	float water_height;
	GLuint texture_player[4];
	GLuint texture_background;
	GLuint texture_water;
	GLuint texture_ground;
	GLuint texture_ladder;
	GLuint texture_crate;
	GLuint texture_powerups;
	Actors actors;
	Platforms platforms;
	Ladders ladders;
	Crates crates;
	Bridges bridges;
	Powerups powerups;
	Countdown timer_powerup;
	GameMode game;
};
