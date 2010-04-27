#pragma once

#include <iostream>
#include <algorithm>
#include <GL/gl.h>
#include <Box2D.h>
#include <boost/ptr_container/ptr_vector.hpp>

#include "util.hh"
#include "texture.hh"
#include "entity.hh"
#include "powerups.hh"
#include "network.hh"

#define PLAYER_RESTITUTION 0.25f
#define PLAYER_FRICTION 0.05f

#define speed_move_ground 2.5f
#define speed_move_airborne (speed_move_ground / 3.0f * 2.0f)
#define speed_move_ladder (speed_move_ground / 3.0f)
#define speed_jump (speed_move_ground * 0.9f)
#define speed_climb speed_jump

class World;

class Actor: public Entity {
  public:
	enum Type { HUMAN, AI, REMOTE } type;

	Actor(GLuint tex = 0, Type t = HUMAN): Entity(tex), type(t),
	  key_up(), key_down(), key_left(), key_right(), key_action(),
	  points(0), dead(false), dir(-1), anim_frame(0), airborne(true), ladder(LADDER_NO), jumping(0), jump_dir(0),
	  wallpenalty(0), powerup(), invisible(false), doublejump(DJUMP_DISALLOW), reversecontrols(false), lograv(false)
	{ }

	void brains() {
		static Countdown actiontimer(1.0);
		static bool stopped = false;
		if (actiontimer()) {
			stopped = false;
			if (randbool() && randbool() && randbool()) stopped = true;
			if (ladder != LADDER_NO && randbool()) jump();
			else move(randbool() ? -1 : 1);
			actiontimer = Countdown(randf(0.5f, 1.5f));
		} else if (!stopped) {
			if (ladder != LADDER_NO && randbool()) jump();
			else move(dir);
		}
	}

	virtual void move(int direction) {
		if (reversecontrols) direction = -direction;
		if (direction != dir) { dir = direction; return; }
		if (direction == dir || can_jump()) {
			// Calc base speed depending on state
			float speed = airborne ? speed_move_airborne : speed_move_ground;
			if (ladder == LADDER_CLIMBING) speed = speed_move_ladder;
			// Apply direction
			speed *= direction;
			// Get old speed
			b2Vec2 v = body->GetLinearVelocity();
			// Determine if jumping direction is already chosen
			if (jump_dir == 0 && airborne && ladder == LADDER_NO && std::abs(v.x) > 0.01f) jump_dir = sign(v.x);
			else if (jump_dir != 0 && airborne && ladder == LADDER_NO && sign(v.x) != sign(jump_dir))
				body->SetLinearVelocity(b2Vec2(0, v.y));
			else if (!airborne || ladder != LADDER_NO) jump_dir = 0;
			// If airborne, only slow down the existing speed if trying to turn
			if (airborne && jump_dir != 0 && direction != jump_dir) {
				body->ApplyForce(b2Vec2(direction * 3, 0), body->GetWorldCenter());
			} else {
				// Don't kill existing higher velocity
				if (direction == dir && std::abs(v.x) > std::abs(speed)) speed = v.x;
				// Set the speed
				//body->SetLinearVelocity(b2Vec2(speed, v.y));
				if (std::abs(v.x) < std::abs(speed)) body->ApplyForce(b2Vec2(speed * 5, 0), body->GetWorldCenter());
			}
		}
		if (airborne) anim_frame = 0;
		else anim_frame = int(GetSecs()*15) % 4;
		dir = direction;
	}

	virtual void stop() {
		if (!airborne || ladder == LADDER_CLIMBING) {
			body->SetLinearVelocity(b2Vec2(0.0f, body->GetLinearVelocity().y));
		}
		anim_frame = 0;
	}

	virtual void jump(bool forcejump = false) {
		if (ladder != LADDER_NO) {
			ladder = LADDER_CLIMBING;
			body->SetLinearVelocity(b2Vec2(0.0f, -speed_climb));
			return;
		}
		if (!can_jump() && !forcejump) return;
		body->SetLinearVelocity(b2Vec2(body->GetLinearVelocity().x, -speed_jump));
		// Handle double jump
		if (airborne && jumping == 0 && doublejump == DJUMP_ALLOW)
			doublejump = DJUMP_JUMPED;
		jumping++;
	}

	virtual void duck() {
		if (ladder == LADDER_YES || ladder == LADDER_CLIMBING) {
			ladder = LADDER_CLIMBING;
			body->SetLinearVelocity(b2Vec2(0.0f, speed_climb));
			return;
		}
	}

	virtual void end_jumping() {
		jumping = 0;
		if (ladder == Actor::LADDER_CLIMBING) {
			body->SetLinearVelocity(b2Vec2(body->GetLinearVelocity().x, 0.0f));
		}
	}

	virtual void action() {
		powerup.action(this);
	}

	void equip(Powerup p) {
		powerup.unequip(this);
		powerup = p;
		powerup.equip(this);
	}

	void unequip() { equip(Powerup()); }

	void die() {
		unequip();
		dead = true;
		points--;
		std::cout << "DEATH! Points: " << points << std::endl;
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

	// Flags / states
	int points;
	bool dead;
	int dir;
	int anim_frame;
	bool airborne;
	enum Ladder { LADDER_NO, LADDER_ROOT, LADDER_YES, LADDER_CLIMBING } ladder;
	int jumping;
	int jump_dir;
	Countdown wallpenalty;
	Powerup powerup;

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
