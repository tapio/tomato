#include <Box2D.h>

#include "world.hh"
#include "player.hh"

void World::addActor(Actor* actor) {
	actors.push_back(actor);
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

	for (GroundBodies::const_iterator it = groundBodies.begin(); it != groundBodies.end(); ++it) {
	}

	for (Actors::const_iterator it = actors.begin(); it != actors.end(); ++it) {
		(*it)->draw();
	}
}
