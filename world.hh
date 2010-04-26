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

#define GRAVITY 45.0f

class Client;

class World {
  public:
	World(int width, int height, TextureMap& tm, bool master = true);

	Actor* shoot(const Actor& shooter);
	bool safe2spawn(float x, float y) const;
	b2Vec2 randomSpawn() const;
	b2Vec2 randomSpawnLocked() const;

	void addMine(float x, float y);
	void addActor(float x, float y, Actor::Type type, GLuint tex = 0, Client* client = NULL);
	bool addPlatform(float x, float y, float w);
	void addLadder(float x, float y, float h);
	void addCrate(float x, float y);
	void addBridge(Platform& leftAnchor, Platform& rightAnchor);
	void addPowerup(float x, float y, Powerup::Type type);

	void generate();

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
	int w;
	int h;
	b2Vec2 view_topleft;
	b2Vec2 view_bottomright;
	int tilesize;
	int water_height;
	GLuint texture_player;
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
};
