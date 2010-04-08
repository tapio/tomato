#pragma once

#include <GL/gl.h>
#include <Box2D.h>

#include "util.hh"
#include "world.hh"

class Player {
  public:
	Player(World& world, float x = 0.0f, float y = 0.0f): x(x), y(y), dir(-1), color(1.0f, 0.0f, 0.0f)
	{
		body = world.addActor(this);

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

	void draw() const {
		float s = 20;
		glColor4fv(color);
		glBegin(GL_TRIANGLE_STRIP);
			glVertex2f(x-s, y+s);
			glVertex2f(x-s, y-s);
			glVertex2f(x+s, y+s);
			glVertex2f(x+s, y-s);
		glEnd();
	}

	float x;
	float y;
	int dir;

  private:

	b2Body* body;
	Color color;

};
