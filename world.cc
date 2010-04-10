#include <cmath>
#include <Box2D.h>

#include "world.hh"
#include "player.hh"
#include "util.hh"
#include "texture.hh"


void World::addActor(float x, float y, GLuint tex) {
	Actor actor(tex);
	// Define the dynamic body. We set its position and call the body factory.
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(x, y);
	bodyDef.fixedRotation = true;
	actor.body = world.CreateBody(&bodyDef);
	//world.addActor(this);

	// Define a circle shape for our dynamic body.
	b2CircleShape circle;
	circle.m_radius = actor.getSize();

	// Define the dynamic body fixture.
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &circle;
	fixtureDef.density = 1.0f; // Set the density to be non-zero, so it will be dynamic.
	fixtureDef.friction = 1.0f; // Stronger friction
	fixtureDef.restitution = 0.25f; // A little bounciness

	// Add the shape to the body.
	actor.getBody()->CreateFixture(&fixtureDef);

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
	fixtureDef.friction = 1.0f;
	p.body->CreateFixture(&fixtureDef);

	platforms.push_back(p);
}


void World::generate() {
	for (int i = 0; i < 10; i++) {
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

	// Update actors' airborne status
	for (Actors::iterator it = actors.begin(); it != actors.end(); ++it) {
		it->airborne = true;
		for (b2ContactEdge* ce = it->getBody()->GetContactList(); ce && ce->contact; ce = ce->next) {
			it->airborne = false; break;
		}
	}
}


void World::draw() const {
	// Background
	int texsize = 512;
	for (int j = 0; j < h/texsize + 1; j++) {
		for (int i = 0; i < w/texsize + 1; i++) {
			float xx = i * texsize;
			float yy = j * texsize;
			float verts[] = { xx, yy + texsize,
			                  xx, yy,
			                  xx + texsize, yy + texsize,
			                  xx + texsize, yy };
			drawVertexArray(&verts[0], &tex_square[0], 4, texture_background);
		}
	}
	// Platforms
	for (Platforms::const_iterator it = platforms.begin(); it != platforms.end(); ++it) {
		it->draw();
	}
	// Players
	for (Actors::const_iterator it = actors.begin(); it != actors.end(); ++it) {
		it->draw();
	}
}
