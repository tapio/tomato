#include <cmath>
#include <Box2D.h>

#include "world.hh"
#include "player.hh"

void World::addActor(Actor* actor) {
	actors.push_back(actor);
}


void World::addPlatform(float x, float y, float w) {
	float h = 1;
	int tilesize = 32;
	Platform p(w, h, texture_ground, 0, tilesize);
	// Create body
	b2BodyDef bodyDef;
	bodyDef.position.Set(x, y);
	p.body = world.CreateBody(&bodyDef);
	// Create shape
	b2PolygonShape box;
	box.SetAsBox(w/2*tilesize, h/2*tilesize);
	// Create fixture
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &box;
	p.body->CreateFixture(&fixtureDef);

	platforms.push_back(p);
}


void World::generate() {
	for (int i = 0; i < 5; i++) {
		addPlatform(randint(0,w), randint(0,h), randint(2,6));
	}
}


void World::update() {
	// Prepare for simulation. Typically we use a time step of 1/60 of a
	// second (60Hz) and 10 iterations. This provides a high quality simulation
	// in most game scenarios.
	float32 timeStep = 1.0f / 60.0f;
	int32 velocityIterations = 6;
	int32 positionIterations = 2;

	// Instruct the world to perform a single step of simulation.
	// It is generally best to keep the time step and iterations fixed.
	world.Step(timeStep, velocityIterations, positionIterations);

	// Clear applied body forces. We didn't apply any forces, but you
	// should know about this function.
	world.ClearForces();
}


void World::draw() const {
	// Background
	int texsize = 512;
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture_background);
	for (int j = 0; j < h/texsize + 1; j++) {
		for (int i = 0; i < w/texsize + 1; i++) {
			float xx = i * texsize;
			float yy = j * texsize;
			glBegin(GL_TRIANGLE_STRIP);
				glTexCoord2f(0.0f, 0.0f);
				glVertex2f(xx, yy + texsize);
				glTexCoord2f(0.0f, 1.0f);
				glVertex2f(xx, yy);
				glTexCoord2f(1.0f, 0.0f);
				glVertex2f(xx + texsize, yy + texsize);
				glTexCoord2f(1.0f, 1.0f);
				glVertex2f(xx + texsize, yy);
			glEnd();
		}
	}
	glDisable(GL_TEXTURE_2D);
	// Platforms
	for (Platforms::const_iterator it = platforms.begin(); it != platforms.end(); ++it) {
		it->draw();
	}
	// Players
	for (Actors::const_iterator it = actors.begin(); it != actors.end(); ++it) {
		(*it)->draw();
	}
}
