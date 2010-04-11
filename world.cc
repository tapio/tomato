#include <cmath>
#include <Box2D.h>

#include "world.hh"
#include "player.hh"
#include "util.hh"
#include "texture.hh"
#include "powerups.hh"


namespace {
	enum ElementType { NONE, PLATFORM, LADDER };
	static ElementType ElementTypes[] = { NONE, PLATFORM, LADDER };
}

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
	fixtureDef.density = 2.0f; // Set the density to be non-zero, so it will be dynamic.
	fixtureDef.friction = 0.25f; // Friction
	fixtureDef.restitution = 0.25f; // A little bounciness

	// Add the shape to the body.
	actor.getBody()->CreateFixture(&fixtureDef);

	actors.push_back(actor);
}


void World::addPlatform(float x, float y, float w) {
	Platform p(w, texture_ground, 0, tilesize);
	// Create body
	b2BodyDef bodyDef;
	bodyDef.position.Set(x + w/2 * tilesize, y + tilesize*0.5f);
	p.body = world.CreateBody(&bodyDef);
	p.body->SetUserData(&ElementTypes[PLATFORM]);
	// Create shape
	b2PolygonShape box;
	box.SetAsBox(w/2*tilesize, 0.5f*tilesize);
	// Create fixture
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &box;
	fixtureDef.friction = 4.0f; // Higher friction
	p.body->CreateFixture(&fixtureDef);

	platforms.push_back(p);
}


void World::addLadder(float x, float y, float h) {
	Ladder l(h, texture_ladder, 0, tilesize);
	// Create body
	b2BodyDef bodyDef;
	bodyDef.position.Set(x + tilesize*0.5f, y + h/2 * tilesize);
	l.body = world.CreateBody(&bodyDef);
	l.body->SetUserData(&ElementTypes[LADDER]);
	// Create shape
	b2PolygonShape laddershape;
	//laddershape.SetAsEdge(b2Vec2(0.5f*tilesize, y), b2Vec2(0.5f*tilesize, h));
	laddershape.SetAsBox(0.10f*tilesize, h/2*tilesize);
	// Create fixture
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &laddershape;
	fixtureDef.isSensor = true; // No collision response
	l.body->CreateFixture(&fixtureDef);

	ladders.push_back(l);
}


void World::addPowerup(float x, float y, Powerup::Type type) {
	Powerup pw(type, texture_powerups);
	// Define the dynamic body. We set its position and call the body factory.
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(x, y);
	bodyDef.fixedRotation = true;
	pw.body = world.CreateBody(&bodyDef);

	// Define a circle shape for our dynamic body.
	b2PolygonShape box;
	box.SetAsBox(pw.getSize(), pw.getSize());

	// Define the dynamic body fixture.
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &box;
	fixtureDef.density = 0.1f; // Set the density to be non-zero, so it will be dynamic.
	fixtureDef.restitution = 1.0f; // Full bounciness
	pw.getBody()->CreateFixture(&fixtureDef);

	// Set a random velocity
	float a = randf(0.0f, 2*PI);
	float spd = randf(GRAVITY*0.5f, GRAVITY*1.5f);
	pw.getBody()->SetLinearVelocity(b2Vec2(cos(a)*spd, sin(a)*spd));

	powerups.push_back(pw);
}


void World::generate() {
	// Create starting platforms
	float x = randf(1.5*tilesize, 2.5*tilesize);
	float y1 = randf(3*tilesize, 5*tilesize);
	float y2 = randf(h-8*tilesize, h-5*tilesize);
	int ytilediff = int((y2-y1) / tilesize) + 1;
	addPlatform(x + tilesize, y1, randint(2,4)); // Top left
	addPlatform(x, y2, randint(2,4)); // Bottom left
	addLadder(x, y2 - ytilediff * tilesize - 1, ytilediff); // Connect with ladder

	float w1 = randint(2,4);
	float w2 = randint(2,4);
	x = randf(w - 2.5*tilesize - tilesize, w - 1.5*tilesize - tilesize);
	y1 = randf(3*tilesize,5*tilesize);
	y2 = randf(h-8*tilesize,h-5*tilesize);
	ytilediff = int((y2-y1) / tilesize) + 1;
	addPlatform(x - w1*tilesize - tilesize, y1, w1); // Top right
	addPlatform(x - w2*tilesize, y2, w2); // Bottom right
	addLadder(x - tilesize, y2 - ytilediff * tilesize - 1, ytilediff); // Connect with ladder
	// Create rest
	for (int i = 0; i < 5; i++) {
		addPlatform(randint(0,w), randint(0,h), randint(2,6));
	}
	//for (int i = 0; i < 4; i++) {
		//addLadder(randint(0,w), randint(0,h), randint(2,6));
	//}
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

	// Update actors' airborne etc. status + gravity
	for (Actors::iterator it = actors.begin(); it != actors.end(); ++it) {
		it->airborne = true;
		it->climbing = Player::NO;
		for (b2ContactEdge* ce = it->getBody()->GetContactList(); ce && ce->other; ce = ce->next) {
			if (ce->other->GetUserData() && *(static_cast<ElementType*>(ce->other->GetUserData())) == LADDER)
				it->climbing = Player::YES;
			else it->airborne = false;
		}
		if (it->climbing == Player::YES && !it->airborne) it->climbing = Player::ROOT;
		// Gravity;
		b2Body* b = it->getBody();
		b->ApplyForce(b2Vec2(0, b->GetMass() * GRAVITY), b->GetWorldCenter());
	}

	// Create power-ups
	if (timer_powerup()) {
		srand(time(NULL));
		float offset = 50.0f;
		addPowerup(randf(offset, w-offset), randf(offset, h-offset), PowerupTypes[randint(Powerup::POWERUPS)]);
		timer_powerup = Countdown(randf(5.0f, 8.0f));
	}
}


void World::draw() const {
	// Background
	int texsize = 256;
	for (int j = 0; j < h/texsize + 1; j++) {
		for (int i = 0; i < w/texsize + 1; i++) {
			float xx = i * texsize;
			float yy = j * texsize;
			float verts[] = { xx, yy + texsize,
			                  xx, yy,
			                  xx + texsize, yy,
			                  xx + texsize, yy + texsize };
			drawVertexArray(&verts[0], &tex_square[0], 4, texture_background);
		}
	}
	// Ladders
	for (Ladders::const_iterator it = ladders.begin(); it != ladders.end(); ++it) {
		it->draw();
	}
	// Platforms
	for (Platforms::const_iterator it = platforms.begin(); it != platforms.end(); ++it) {
		it->draw();
	}
	// Players
	for (Actors::const_iterator it = actors.begin(); it != actors.end(); ++it) {
		it->draw();
	}
	// Power-ups
	for (Powerups::const_iterator it = powerups.begin(); it != powerups.end(); ++it) {
		it->draw();
	}
	// Water
	for (int i = 0; i < w / water_height + 1; i++) {
		float xx = i * water_height;
		float yy = h - water_height;
		float verts[] = { xx, yy + water_height,
						  xx, yy,
						  xx + water_height, yy,
						  xx + water_height, yy + water_height };
		drawVertexArray(&verts[0], &tex_square[0], 4, texture_water);
	}
}
