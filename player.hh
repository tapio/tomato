#pragma once

#include <iostream>
#include <GL/gl.h>
#include <Box2D.h>

#include "util.hh"
#include "world.hh"

class Player {
  public:
	enum Type { HUMAN, AI, REMOTE } type;

	Player(GLuint tex = 0, Type t = HUMAN): type(t), dir(-1), airborne(true), jumping(false), texture(tex), color(1.0f, 0.0f, 0.0f), size(16.0f)
	{ }

	void move(int direction) {
		std::cout << "MOVE " << direction << std::endl;
		body->ApplyLinearImpulse(b2Vec2(20000.0f * direction, 0.0f), body->GetWorldCenter());
	}

	void jump() {
		std::cout << "JUMP, airborne: " << airborne << std::endl;
		jumping = true;
		body->ApplyLinearImpulse(b2Vec2(0.0f, -20000.0f), body->GetWorldCenter());
	}

	void duck() {
		std::cout << "DUCK" << std::endl;
	}

	void action() {
		std::cout << "ACTION" << std::endl;
	}

	void draw() const {
		float x = getX(), y = getY();
		//glColor4fv(color);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texture);
		glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(0.0f, 0.0f);
			glVertex2f(x-size, y+size);
			glTexCoord2f(0.0f, 1.0f);
			glVertex2f(x-size, y-size);
			glTexCoord2f(1.0f, 0.0f);
			glVertex2f(x+size, y+size);
			glTexCoord2f(1.0f, 1.0f);
			glVertex2f(x+size, y-size);
		glEnd();
		glDisable(GL_TEXTURE_2D);
	}

	float32 getX() const { return body->GetPosition().x; }
	float32 getY() const { return body->GetPosition().y; }
	b2Body* getBody() { return body; }
	float getSize() { return size; }
	bool can_jump() const { return !airborne || jumping; }

	int dir;

	int KEY_UP;
	int KEY_DOWN;
	int KEY_LEFT;
	int KEY_RIGHT;
	int KEY_ACTION;

	bool airborne;
	bool jumping;

	b2Body* body;

  private:

	GLuint texture;
	Color color;
	float size;
};

typedef std::vector<Player> Players;
