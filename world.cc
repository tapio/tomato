#include <cmath>
#include <Box2D.h>

#include "world.hh"
#include "player.hh"
#include "util.hh"
#include "texture.hh"
#include "powerups.hh"


namespace {
	enum ElementType { NONE, PLATFORM, LADDER, CRATE, POWERUP, ACTOR };
	static ElementType ElementTypes[] = { NONE, PLATFORM, LADDER, CRATE, POWERUP, ACTOR };
	struct WorldElement {
		WorldElement(ElementType type, void* element = NULL): type(type), ptr(element) { }
		ElementType type;
		void* ptr;
	};
}

void World::addActor(float x, float y, GLuint tex) {
	Actor actor(tex);
	// Define the dynamic body. We set its position and call the body factory.
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(x, y);
	bodyDef.fixedRotation = true;
	actor.body = world.CreateBody(&bodyDef);
	actor.body->SetUserData(&ElementTypes[ACTOR]);
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


void World::addCrate(float x, float y) {
	Crate cr(texture_crate, 0, tilesize);
	// Define the dynamic body. We set its position and call the body factory.
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(x, y);
	cr.body = world.CreateBody(&bodyDef);
	cr.body->SetUserData(&ElementTypes[CRATE]);

	// Define a shape for our dynamic body.
	b2PolygonShape box;
	box.SetAsBox(0.5f*tilesize, 0.5f*tilesize);

	// Define the dynamic body fixture.
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &box;
	fixtureDef.density = 1.0f; // Set the density to be non-zero, so it will be dynamic.
	fixtureDef.restitution = 0.05f;
	cr.getBody()->CreateFixture(&fixtureDef);

	crates.push_back(cr);
}


void World::addPowerup(float x, float y, Powerup::Type type) {
	PowerupEntity pw(type, texture_powerups);
	// Define the dynamic body. We set its position and call the body factory.
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(x, y);
	bodyDef.fixedRotation = true;
	pw.body = world.CreateBody(&bodyDef);
	pw.body->SetUserData(&ElementTypes[POWERUP]);

	// Define a circle shape for our dynamic body.
	b2CircleShape shape;
	shape.m_radius = pw.getSize();

	// Define the dynamic body fixture.
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &shape;
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
	float xoff = 1.5*tilesize;
	float yoff = 2.5*tilesize;
	// Create starting platforms
	float x = randf(xoff, xoff+tilesize);
	float y1 = randf(3*tilesize, 5*tilesize);
	float y2 = randf(h-8*tilesize, h-5*tilesize);
	int ytilediff = int((y2-y1) / tilesize) + 1;
	addPlatform(x + tilesize, y1, randint(2,4)); // Top left
	addPlatform(x, y2, randint(2,4)); // Bottom left
	addLadder(x, y2 - ytilediff * tilesize - 1, ytilediff); // Connect with ladder

	float w1 = randint(2,4);
	float w2 = randint(2,4);
	x = randf(w - xoff - tilesize - tilesize, w - xoff - tilesize);
	y1 = randf(3*tilesize,5*tilesize);
	y2 = randf(h-8*tilesize,h-5*tilesize);
	ytilediff = int((y2-y1) / tilesize) + 1;
	addPlatform(x - w1*tilesize - tilesize, y1, w1); // Top right
	addPlatform(x - w2*tilesize, y2, w2); // Bottom right
	addLadder(x - tilesize, y2 - ytilediff * tilesize - 1, ytilediff); // Connect with ladder
	// Create rest of platforms
	for (int j = yoff; j < h - water_height - yoff; j += 4*tilesize) {
		for (int i = xoff + 6*tilesize; i < w - xoff - 6*tilesize; i += 7*tilesize) {
			addPlatform(i + randf(-2*tilesize,2*tilesize), j + randf(tilesize,tilesize), randint(2,6));
		}
	}
	//for (int i = 0; i < 4; i++) {
		//addLadder(randint(0,w), randint(0,h), randint(2,6));
	//}
	// Create crates
	for (int i = 0; i < 15; i++) {
		addCrate(randint(0,w), randint(0,h));
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

	float offset = 50.0f; // For spawning things away from borders

	// Update actors' airborne etc. status + gravity
	for (Actors::iterator it = actors.begin(); it != actors.end(); ++it) {
		it->airborne = true;
		it->climbing = Actor::NO;
		// Water
		if (it->getBody()->GetWorldCenter().y + it->getSize() >= h - water_height) it->die();
		// Death
		if (it->is_dead()) {
			srand(time(NULL));
			it->getBody()->SetLinearVelocity(b2Vec2());
			it->getBody()->SetTransform(b2Vec2(randf(offset, w-offset), randf(offset, h-offset)), 0);
			it->dead = false;
			continue;
		}
		if (it->powerup.expired()) it->unequip();
		// Check for contacts
		for (b2ContactEdge* ce = it->getBody()->GetContactList(); ce && ce->other; ce = ce->next) {
			// Ladders
			if (ce->other->GetUserData() && *(static_cast<ElementType*>(ce->other->GetUserData())) == LADDER)
				it->climbing = Actor::YES;
			// Power-ups
			else if (ce->other->GetUserData() && *(static_cast<ElementType*>(ce->other->GetUserData())) == POWERUP) {
				ce->other->SetUserData(NULL);
				// Find the powerup
				for (Powerups::iterator pu = powerups.begin(); pu != powerups.end(); ++pu) {
					if (!pu->getBody()->GetUserData()) {
						it->equip(pu->effect);
						world.DestroyBody(ce->other);
						powerups.erase(pu);
						break;
					}
				}
			// Other players
			} else if (ce->other->GetUserData() && *(static_cast<ElementType*>(ce->other->GetUserData())) == ACTOR) {
				ce->other->SetUserData(NULL);
				// Find the actor
				for (Actors::iterator ac = actors.begin(); ac != actors.end(); ++ac) {
					if (!ac->getBody()->GetUserData()) {
						it->powerup.touch(&(*it), &(*ac));
						break;
					}
				}
				ce->other->SetUserData(&ElementTypes[ACTOR]);
			// Ground
			} else it->airborne = false;
		}
		if (!it->airborne) {
			if (it->climbing == Actor::YES) it->climbing = Actor::ROOT;
			if (it->doublejump == DJUMP_JUMPED) it->doublejump = DJUMP_ALLOW;
		}
		// Gravity;
		b2Body* b = it->getBody();
		b->ApplyForce(b2Vec2(0, b->GetMass() * GRAVITY), b->GetWorldCenter());
	}
	// Gravity for crates
	for (Crates::iterator it = crates.begin(); it != crates.end(); ++it) {
		b2Body* b = it->getBody();
		b->ApplyForce(b2Vec2(0, b->GetMass() * GRAVITY), b->GetWorldCenter());
	}
	// Create power-ups
	if (timer_powerup()) {
		srand(time(NULL));
		addPowerup(randf(offset, w-offset), randf(offset, h-offset), Powerup::Random());
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
	// Crates
	for (Crates::const_iterator it = crates.begin(); it != crates.end(); ++it) {
		glPushMatrix();
		glTranslatef(it->getBody()->GetWorldCenter().x, it->getBody()->GetWorldCenter().y, 0);
		glRotatef(it->getBody()->GetAngle() * 180.0 / PI, 0, 0, 1);
		glTranslatef(-it->getBody()->GetWorldCenter().x, -it->getBody()->GetWorldCenter().y, 0);
		it->draw();
		glPopMatrix();
	}
	// Players
	for (Actors::const_iterator it = actors.begin(); it != actors.end(); ++it) {
		if (!it->invisible && !it->is_dead()) it->draw();
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
