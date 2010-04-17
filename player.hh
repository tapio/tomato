#pragma once

#include <iostream>
#include <algorithm>
#include <GL/gl.h>
#include <Box2D.h>

#include "util.hh"
#include "texture.hh"
#include "entity.hh"
#include "powerups.hh"

class Actor: public Entity {
  public:
	enum Type { HUMAN, AI, REMOTE } type;

	Actor(GLuint tex = 0, Type t = HUMAN): Entity(16.0f, tex),
	  type(t), points(0), dead(false), dir(-1), anim_frame(0), airborne(true), climbing(NO), jumping(0), powerup(),
	  invisible(false), doublejump(DJUMP_DISALLOW), reversecontrols(false)
	{ }

	void move(int direction) {
		if (reversecontrols) direction = -direction;
		if (direction != dir) { dir = direction; return; }
		if (direction == dir || can_jump()) {
			// Calc base speed depending on state
			float speed = airborne ? 20.0f : 30.0f;
			if (climbing == YES) speed = 10.0f;
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
		if (!airborne) {
			body->SetLinearVelocity(b2Vec2(0.0f, body->GetLinearVelocity().y));
		}
	}

	void jump(bool forcejump = false) {
		std::cout << "JUMP, airborne: " << airborne << ", climbing: " << climbing << ", djump: " << doublejump << std::endl;
		if (climbing != NO) {
			body->SetLinearVelocity(b2Vec2(climbing == YES ? 0.0f : body->GetLinearVelocity().x, -30.0f));
			return;
		}
		if (!can_jump() && !forcejump) return;
		body->SetLinearVelocity(b2Vec2(body->GetLinearVelocity().x, -30.0f));
		// Handle double jump
		if (airborne && jumping == 0 && doublejump == DJUMP_ALLOW)
			doublejump = DJUMP_JUMPED;
		jumping++;
	}

	void duck() {
		if (climbing == YES) {
			body->SetLinearVelocity(b2Vec2(body->GetLinearVelocity().x, 30.0f));
			return;
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
	enum Climb { NO, ROOT, YES } climbing;
	int jumping;

	Powerup powerup;

	// Power-up attributes
	bool invisible;
	DoubleJumpStatus doublejump;
	bool reversecontrols;
};

typedef std::vector<Actor> Actors;
typedef std::vector<Actor> Players;
