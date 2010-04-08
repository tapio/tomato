#pragma once

#include <vector>
#include <Box2D.h>

class Player;

typedef Player Actor;

class World {
  public:
	World(int width, int height): world(b2Vec2(0.0f, 2.0f), true), w(width), h(height)
	{
		// Define the ground body.
		//b2BodyDef groundBodyDef;
		//groundBodyDef.position.Set(0.0f, -10.0f);

		// Call the body factory which allocates memory for the ground body
		// from a pool and creates the ground box shape (also from a pool).
		// The body is also added to the world.
		//b2Body* groundBody = world.CreateBody(&groundBodyDef);

		// Define the ground box shape.
		//b2PolygonShape groundBox;

		// The extents are the half-widths of the box.
		//groundBox.SetAsBox(w/2, 10.0f);

		// Add the ground fixture to the ground body.
		//groundBody->CreateFixture(&groundBox, 0.0f);

	}

	void addActor(Actor* actor);

	void update();

	void draw() const;

	b2World& getWorld() { return world; }

  private:

	b2World world;
	int w;
	int h;
	typedef std::vector<Actor*> Actors;
	Actors actors;
	typedef std::vector<b2Body*> GroundBodies;
	GroundBodies groundBodies;

};
