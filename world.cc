#include <Box2D.h>

#include "world.hh"
#include "player.hh"

b2Body* World::addActor(Actor* actor) {
	actors.push_back(actor);
	// Define the dynamic body. We set its position and call the body factory.
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(actor->x, actor->y);
	return world.CreateBody(&bodyDef);
}


void World::draw() const {

	for (GroundBodies::const_iterator it = groundBodies.begin(); it != groundBodies.end(); ++it) {
	}

	for (Actors::const_iterator it = actors.begin(); it != actors.end(); ++it) {
		(*it)->draw();
	}
}
