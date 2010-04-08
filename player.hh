#pragma once

#include <iostream>
#include <GL/gl.h>
#include <Box2D.h>

#include "util.hh"
#include "world.hh"

class Player {
  public:
	enum Type { HUMAN, AI, REMOTE } type;

	Player(World& world, float x = 0.0f, float y = 0.0f, GLuint tex = 0, Type t = HUMAN): type(t), dir(-1), texture(tex), color(1.0f, 0.0f, 0.0f), size(16.0f)
	{
		// Define the dynamic body. We set its position and call the body factory.
		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		bodyDef.position.Set(x, y);
		bodyDef.fixedRotation = true;
		body = world.getWorld().CreateBody(&bodyDef);
		world.addActor(this);

		// Define a circle shape for our dynamic body.
		b2CircleShape circle;
		circle.m_radius = size;

		// Define the dynamic body fixture.
		b2FixtureDef fixtureDef;
		fixtureDef.shape = &circle;

		// Set the density to be non-zero, so it will be dynamic.
		fixtureDef.density = 1.0f;

		// Add the shape to the body.
		body->CreateFixture(&fixtureDef);
	}

	void move(int direction) {
		std::cout << "MOVE " << direction << std::endl;
		body->ApplyLinearImpulse(b2Vec2(20000.0f * direction, 0.0f), body->GetWorldCenter());
	}

	void jump() {
		std::cout << "JUMP" << std::endl;
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

	int dir;

	int KEY_UP;
	int KEY_DOWN;
	int KEY_LEFT;
	int KEY_RIGHT;
	int KEY_ACTION;

  private:

	b2Body* body;
	GLuint texture;
	Color color;
	float size;
};

typedef std::vector<Player> Players;
