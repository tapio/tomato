#pragma once

#include <iostream>
#include <algorithm>
#include <GL/gl.h>
#include <Box2D.h>

#include "util.hh"
#include "texture.hh"
#include "entity.hh"
#include "powerups.hh"

#define PLAYER_RESTITUTION 0.25f

#define speed_move_ground 55.0f
#define speed_move_airborne (speed_move_ground / 3.0f * 2.0f)
#define speed_move_ladder (speed_move_ground / 3.0f)
#define speed_jump (speed_move_ground * 0.9f)
#define speed_climb speed_jump

class World;

class Actor: public Entity {
  public:
	enum Type { HUMAN, AI, REMOTE } type;

	Actor(GLuint tex = 0, Type t = HUMAN): Entity(16.0f, tex),
	  type(t), points(0), dead(false), dir(-1), anim_frame(0), airborne(true), ladder(LADDER_NO), jumping(0), powerup(),
	  invisible(false), doublejump(DJUMP_DISALLOW), reversecontrols(false), lograv(false)
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

	void move(int direction) {
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
			// If airborne, only slow down the existing speed if trying to turn
			if (airborne && direction != sign(v.x) && std::abs(v.x) > 0.01f) speed = v.x * 0.9;
			// Don't kill existing higher velocity
			else if (direction == dir && std::abs(v.x) > std::abs(speed)) speed = v.x;
			body->SetLinearVelocity(b2Vec2(speed, v.y));
		}
		anim_frame = (anim_frame + 1) % 4;
		dir = direction;
	}

	void stop() {
		if (!airborne || ladder == LADDER_CLIMBING) {
			body->SetLinearVelocity(b2Vec2(0.0f, body->GetLinearVelocity().y));
		}
	}

	void jump(bool forcejump = false) {
		std::cout << "JUMP, airborne: " << airborne << ", climbing: " << ladder << ", djump: " << doublejump << std::endl;
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

	void duck() {
		if (ladder == LADDER_YES || ladder == LADDER_CLIMBING) {
			ladder = LADDER_CLIMBING;
			body->SetLinearVelocity(b2Vec2(0.0f, speed_climb));
			return;
		}
	}

	void end_jumping() {
		jumping = 0;
		if (ladder == Actor::LADDER_CLIMBING) {
			body->SetLinearVelocity(b2Vec2(body->GetLinearVelocity().x, 0.0f));
		}
	}

	void action() {
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

	bool is_dead() const { return dead; }
	bool can_jump() const { return !airborne || (jumping > 0 && jumping < 5)
		|| (jumping == 0 && doublejump == DJUMP_ALLOW); }

	int KEY_UP;
	int KEY_DOWN;
	int KEY_LEFT;
	int KEY_RIGHT;
	int KEY_ACTION;

	// Flags / states
	int points;
	bool dead;
	int dir;
	int anim_frame;
	bool airborne;
	enum Ladder { LADDER_NO, LADDER_ROOT, LADDER_YES, LADDER_CLIMBING } ladder;
	int jumping;

	Powerup powerup;

	// Power-up attributes
	bool invisible;
	DoubleJumpStatus doublejump;
	bool reversecontrols;
	bool lograv;
};

typedef std::vector<Actor> Actors;
typedef std::vector<Actor> Players;
