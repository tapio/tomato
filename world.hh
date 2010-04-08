#pragma once

#include <vector>
#include <Box2D.h>

class Player;

typedef Player Actor;

class World {
  public:
	World(int width, int height): world(b2Vec2(0.0f, -10.0f), true), w(width), h(height)
	{

		// Define the ground body.
		b2BodyDef groundBodyDef;
		groundBodyDef.position.Set(0.0f, -10.0f);

		// Call the body factory which allocates memory for the ground body
		// from a pool and creates the ground box shape (also from a pool).
		// The body is also added to the world.
		b2Body* groundBody = world.CreateBody(&groundBodyDef);

		// Define the ground box shape.
		b2PolygonShape groundBox;

		// The extents are the half-widths of the box.
		groundBox.SetAsBox(w/2, 10.0f);

		// Add the ground fixture to the ground body.
		groundBody->CreateFixture(&groundBox, 0.0f);

	}

	b2Body* addActor(Actor* actor);

	void update() {
		// Prepare for simulation. Typically we use a time step of 1/60 of a
		// second (60Hz) and 10 iterations. This provides a high quality simulation
		// in most game scenarios.
		float32 timeStep = 1.0f / 60.0f;
		int32 velocityIterations = 6;
		int32 positionIterations = 2;

		// This is our little game loop.
		for (int32 i = 0; i < 60; ++i) {
			// Instruct the world to perform a single step of simulation.
			// It is generally best to keep the time step and iterations fixed.
			world.Step(timeStep, velocityIterations, positionIterations);

			// Clear applied body forces. We didn't apply any forces, but you
			// should know about this function.
			world.ClearForces();

			// Now print the position and angle of the body.
			//b2Vec2 position = body->GetPosition();
			//float32 angle = body->GetAngle();
		}
	}

	void draw() const;

  private:

	b2World world;
	int w;
	int h;
	typedef std::vector<Actor*> Actors;
	Actors actors;
	typedef std::vector<b2Body*> GroundBodies;
	GroundBodies groundBodies;

};
