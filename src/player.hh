#pragma once

#include <iostream>
#include <algorithm>
#include <GL/gl.h>
#include <Box2D.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/noncopyable.hpp>

#include "util.hh"
#include "texture.hh"
#include "entity.hh"
#include "powerups.hh"
#include "network.hh"

#define PLAYER_RESTITUTION 0.25f
#define PLAYER_FRICTION 0.2f

#define speed_move_ground 2.5f
#define speed_move_airborne (speed_move_ground / 3.0f * 2.0f)
#define speed_move_ladder (speed_move_ground / 3.0f)
#define speed_jump (speed_move_ground * 0.9f)
#define speed_climb speed_jump

#define NAMES 4

struct Points {
	Points(): wins(0), total_score(0), round_score(0), kills(0), deaths(0) { }
	void add(int howmuch) { total_score += howmuch; round_score += howmuch; }
	int wins;
	int total_score;
	int round_score;
	int kills;
	int deaths;
};

class World;

class Actor: public boost::noncopyable, public Entity {
	static int ref_count;
	static int human_count;

  public:
	enum Type { HUMAN, AI, REMOTE } const type;
	static const std::string Names[NAMES];

	Actor(GLuint tex = 0, Type t = HUMAN): Entity(tex), type(t),
	  key_up(), key_down(), key_left(), key_right(), key_action(),
	  points(), dead(false), dir(-1), anim_frame(0), airborne(true), ladder(LADDER_NO), jumping(0), jump_dir(0),
	  wallpenalty(0), powerup(), respawn(), invisible(false), doublejump(DJUMP_DISALLOW), reversecontrols(false), lograv(false)
	{
		name = Names[ref_count % NAMES];
		++ref_count;
		if (type == HUMAN) {
			keys_id = human_count;
			++human_count;
		}
	}

	~Actor() {
		--ref_count;
		if (type == HUMAN) --human_count;
	}

	void brains();

	virtual void move(int direction);
	virtual void jump(bool forcejump = false);
	virtual void duck();
	virtual void stop();
	virtual void end_jumping();
	virtual void action() { powerup.action(this); }

	void equip(Powerup p) {
		powerup.unequip(this);
		powerup = p;
		powerup.equip(this);
	}

	void unequip() { equip(Powerup()); }

	void die() {
		unequip();
		dead = true;
		// Reset key states
		key_up = false; key_down = false; key_left = false; key_right = false; key_action = false;
	}

	virtual void draw() const { Entity::draw(anim_frame, 4, dir < 0); }

	virtual SerializedEntity serialize() const {
		SerializedEntity se = Entity::serialize();
		se.type = dir;
		return se;
	}

	virtual void unserialize(std::string data) {
		Entity::unserialize(data);
		SerializedEntity* se = reinterpret_cast<SerializedEntity*>(&data[0]);
		dir = se->type;
	}

	bool is_dead() const { return dead; }
	bool can_jump() const { return !airborne || (jumping > 0 && jumping < 5)
		|| (jumping == 0 && doublejump == DJUMP_ALLOW); }

	std::string getName() const { return name; }

	void key_state(int k, bool pressed);
	void handle_keys();

	int KEY_UP;
	int KEY_DOWN;
	int KEY_LEFT;
	int KEY_RIGHT;
	int KEY_ACTION;

	bool key_up;
	bool key_down;
	bool key_left;
	bool key_right;
	bool key_action;

	int keys_id;
	std::string name;

	// Flags / states
	Points points;
	bool dead;
	int dir;
	int anim_frame;
	bool airborne;
	enum Ladder { LADDER_NO, LADDER_ROOT, LADDER_YES, LADDER_CLIMBING } ladder;
	int jumping;
	int jump_dir;
	Countdown wallpenalty;
	Powerup powerup;
	Countdown respawn;

	// Power-up attributes
	bool invisible;
	DoubleJumpStatus doublejump;
	bool reversecontrols;
	bool lograv;
};

typedef boost::ptr_vector<Actor> Actors;
typedef boost::ptr_vector<Actor> Players;


class OnlinePlayer: public Actor {
  public:

	// These are sent to the server indicating player's action
	enum Action {
		MOVE_LEFT = 1,
		MOVE_RIGHT,
		JUMP,
		DUCK,
		STOP_MOVING,
		STOP_JUMPING,
		ACTION
	};

	OnlinePlayer(Client* client, GLuint tex = 0, Type t = HUMAN): Actor(tex, t), client(client) { }

#ifdef USE_NETWORK

	virtual void move(int direction) {
		if (direction < 0) client->send(MOVE_LEFT);
		else if (direction > 0) client->send(MOVE_RIGHT);
		Actor::move(direction);
	}

	virtual void stop() {
		client->send(STOP_MOVING);
		Actor::stop();
	}

	virtual void jump(bool forcejump = false) {
		client->send(JUMP);
		Actor::jump(forcejump);
	}

	virtual void duck() {
		client->send(DUCK);
		Actor::duck();
	}

	virtual void end_jumping() {
		client->send(STOP_JUMPING);
		Actor::end_jumping();
	}

	virtual void action() {
		client->send(ACTION);
		// Don't do the action locally, it probably just breaks things
	}

#endif

  private:

	Client* client;

};
