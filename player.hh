#pragma once

#include <iostream>
#include <algorithm>
#include <GL/gl.h>
#include <Box2D.h>

#include "util.hh"
#include "world.hh"
#include "texture.hh"

class Player {
  public:
	enum Type { HUMAN, AI, REMOTE } type;

	Player(GLuint tex = 0, Type t = HUMAN): type(t), dir(-1), anim_frame(0), airborne(true), climbing(NO), jumping(0), texture(tex), color(1.0f, 0.0f, 0.0f), size(16.0f)
	{ }

	void move(int direction) {
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
		std::cout << "JUMP, airborne: " << airborne << ", climbing: " << climbing <<std::endl;
		if (climbing != NO) {
			body->SetLinearVelocity(b2Vec2(climbing == YES ? 0.0f : body->GetLinearVelocity().x, -30.0f));
			return;
		}
		if (!can_jump() && !forcejump) return;
		body->SetLinearVelocity(b2Vec2(body->GetLinearVelocity().x, -30.0f));
		jumping++;
	}

	void duck() {
		if (climbing == YES) {
			body->SetLinearVelocity(b2Vec2(body->GetLinearVelocity().x, 30.0f));
			return;
		}
	}

	void action() {
		std::cout << "ACTION" << std::endl;
	}

	void draw() const {
		float x = getX(), y = getY();
		float vc[] = { x-size, y+size,
		               x-size, y-size,
		               x+size, y-size,
		               x+size, y+size };

		drawVertexArray(&vc[0], getTileTexCoords(anim_frame, 4, 4, dir < 0), 4, texture);
	}

	float32 getX() const { return body->GetPosition().x; }
	float32 getY() const { return body->GetPosition().y; }
	b2Body* getBody() { return body; }
	float getSize() { return size; }
	bool can_jump() const { return !airborne || (jumping > 0 && jumping < 5); }

	int dir;
	int anim_frame;

	int KEY_UP;
	int KEY_DOWN;
	int KEY_LEFT;
	int KEY_RIGHT;
	int KEY_ACTION;

	bool airborne;
	enum Climb { NO, ROOT, YES } climbing;
	int jumping;

	b2Body* body;

  private:

	GLuint texture;
	Color color;
	float size;
};

typedef std::vector<Player> Players;
