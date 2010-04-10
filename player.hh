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

	Player(GLuint tex = 0, Type t = HUMAN): type(t), dir(-1), anim_frame(0), airborne(true), jumping(0), texture(tex), color(1.0f, 0.0f, 0.0f), size(16.0f)
	{ }

	void move(int direction) {
		if (direction == dir || can_jump()) {
			float speed = airborne ? 25.0f : 50.0f;
			b2Vec2 v = body->GetLinearVelocity();
			if (direction == dir) speed = std::max(speed, v.x); // Don't kill existing higher velocity
			body->SetLinearVelocity(b2Vec2(speed * direction, v.y));
		}
		anim_frame = (anim_frame + 1) % 4;
		dir = direction;
	}

	void stop() {
		if (!airborne) {
			body->SetLinearVelocity(b2Vec2(0.0f, body->GetLinearVelocity().y));
		}
	}

	void jump() {
		std::cout << "JUMP, airborne: " << airborne << std::endl;
		body->ApplyLinearImpulse(b2Vec2(0.0f, -10000.0f), body->GetWorldCenter());
		jumping++;
	}

	void duck() {
		std::cout << "DUCK" << std::endl;
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
	int jumping;

	b2Body* body;

  private:

	GLuint texture;
	Color color;
	float size;
};

typedef std::vector<Player> Players;
