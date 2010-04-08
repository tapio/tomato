#pragma once

#include <iostream>
#include <GL/gl.h>
#include <Box2D.h>

#include "util.hh"
#include "world.hh"

class Player {
  public:
	enum Type { HUMAN, AI, REMOTE } type;

	Player(World& world, float x = 0.0f, float y = 0.0f, Type t = HUMAN): type(t), dir(-1), color(1.0f, 0.0f, 0.0f)
	{
		// Define the dynamic body. We set its position and call the body factory.
		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		bodyDef.position.Set(x, y);
		body = world.getWorld().CreateBody(&bodyDef);
		world.addActor(this);

		// Define another box shape for our dynamic body.
		b2PolygonShape dynamicBox;
		dynamicBox.SetAsBox(1.0f, 1.0f);

		// Define the dynamic body fixture.
		b2FixtureDef fixtureDef;
		fixtureDef.shape = &dynamicBox;

		// Set the box density to be non-zero, so it will be dynamic.
		fixtureDef.density = 1.0f;

		// Override the default friction.
		fixtureDef.friction = 0.3f;

		// Add the shape to the body.
		body->CreateFixture(&fixtureDef);
	}

	void move(int direction) {
		std::cout << "MOVE " << direction << std::endl;
	}

	void jump() {
		std::cout << "JUMP" << std::endl;
	}

	void duck() {
		std::cout << "DUCK" << std::endl;
	}

	void action() {
		std::cout << "ACTION" << std::endl;
	}

	void draw() const {
		float s = 20;
		float x = getX(), y = getY();
		glColor4fv(color);
		glBegin(GL_TRIANGLE_STRIP);
			glVertex2f(x-s, y+s);
			glVertex2f(x-s, y-s);
			glVertex2f(x+s, y+s);
			glVertex2f(x+s, y-s);
		glEnd();
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
	Color color;

};

typedef std::vector<Player> Players;
