#pragma once

#include <vector>
#include <Box2D.h>

class Player;

typedef Player Actor;

class World {
  public:
	World(int width, int height): world(b2Vec2(0.0f, 2.0f), true), w(width), h(height)
	{
		float hw = w*0.5, hh = h*0.5;

		// Define the border bodies
		b2BodyDef borderBodyDef;
		borderBodyDef.position.Set(hw, hh);
		b2Body* borderBody = world.CreateBody(&borderBodyDef);

		// Define the border shapes
		b2PolygonShape borderBoxLeft, borderBoxRight, borderBoxTop, borderBoxBottom;
		borderBoxLeft.SetAsEdge(b2Vec2(-hw,-hh), b2Vec2(-hw,hh));
		borderBoxRight.SetAsEdge(b2Vec2(hw,-hh), b2Vec2(hw,hh));
		borderBoxTop.SetAsEdge(b2Vec2(-hw,-hh), b2Vec2(hw,-hh));
		borderBoxBottom.SetAsEdge(b2Vec2(-hw,hh), b2Vec2(hw,hh));

		// Add the border fixtures to the body
		borderBody->CreateFixture(&borderBoxLeft, 0.0f);
		borderBody->CreateFixture(&borderBoxRight, 0.0f);
		borderBody->CreateFixture(&borderBoxTop, 0.0f);
		borderBody->CreateFixture(&borderBoxBottom, 0.0f);

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
