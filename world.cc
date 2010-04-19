#include <cmath>
#include <GL/glu.h>
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

void World::addActor(float x, float y, Actor::Type type, GLuint tex) {
	Actor actor(tex, type);
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
	fixtureDef.restitution = PLAYER_RESTITUTION; // Bounciness
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
	p.buildVertices();
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
	l.buildVertices();
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
	shape.m_radius = pw.getSize() * 0.75f;

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
	for (int i = 0; i < 8; i++) {
		addCrate(randint(0,w), randint(0,h));
	}

}


void World::update() {
	// Prepare for simulation. Typically we use a time step of 1/60 of a
	// second (60Hz) and 10 iterations. This provides a high quality simulation
	// in most game scenarios.
	float32 timeStep = 1.0f / 60.0f;
	int32 velocityIterations = 6;
	int32 positionIterations = 4;

	#ifdef USE_THREADS
	unsigned int t = SDL_GetTicks();
	{
		boost::mutex::scoped_lock l(mutex);
	#endif

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
			bool climbing = (it->ladder == Actor::LADDER_CLIMBING);
			it->ladder = Actor::LADDER_NO;
			// Water
			if (it->getBody()->GetWorldCenter().y + it->getSize() >= h - water_height) it->die();
			// Death
			if (it->is_dead()) {
				it->getBody()->SetLinearVelocity(b2Vec2());
				it->getBody()->SetTransform(b2Vec2(randf(offset, w-offset), randf(offset, h*0.667)), 0);
				it->dead = false;
				continue;
			}
			if (it->powerup.expired()) it->unequip();
			// Check for contacts
			for (b2ContactEdge* ce = it->getBody()->GetContactList(); ce && ce->other; ce = ce->next) {
				// Ladders
				if (ce->other->GetUserData() && *(static_cast<ElementType*>(ce->other->GetUserData())) == LADDER)
					it->ladder = Actor::LADDER_YES;
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
				if (it->ladder == Actor::LADDER_YES) it->ladder = Actor::LADDER_ROOT;
				if (it->doublejump == DJUMP_JUMPED) it->doublejump = DJUMP_ALLOW;
			}
			if (climbing && it->ladder == Actor::LADDER_YES) it->ladder = Actor::LADDER_CLIMBING;
			// Gravity
			float grav_mult = (it->lograv ? 0.1 : 1.0) * (it->ladder == Actor::LADDER_CLIMBING ? 0.0 : 1.0);
			b2Body* b = it->getBody();
			b->ApplyForce(b2Vec2(0, b->GetMass() * GRAVITY * grav_mult), b->GetWorldCenter());
			// AI
			if (it->type == Actor::AI) it->brains();
		}
		// Crates
		for (Crates::iterator it = crates.begin(); it != crates.end(); ++it) {
			// Respawn if in water
			if (it->getBody()->GetWorldCenter().y - it->getSize() >= h - water_height) {
				it->getBody()->SetLinearVelocity(b2Vec2());
				it->getBody()->SetAngularVelocity(0);
				it->getBody()->SetTransform(b2Vec2(randf(offset, w-offset), randf(offset, h*0.667)), 0);
			}
			// Gravity
			b2Body* b = it->getBody();
			b->ApplyForce(b2Vec2(0, b->GetMass() * GRAVITY), b->GetWorldCenter());
		}
		// Create power-ups
		if (timer_powerup()) {
			addPowerup(randf(offset, w-offset), randf(offset, h-offset), Powerup::Random());
			timer_powerup = Countdown(randf(5.0f, 8.0f));
		}
	#ifdef USE_THREADS
	} // < mutex
	// TODO: Hackish, usleep not available in Windows
	t = SDL_GetTicks() - t;
	t = timeStep * 1000 - t;
	if (t > 0) usleep(t*500);
	#endif
}


void World::draw() const {
	#ifdef USE_THREADS
	boost::mutex::scoped_lock l(mutex);
	#endif
	// Magick zooming camera
	static const float xmargin = 300.0f;
	static const float ymargin = 150.0f;
	float x1 = w, y1 = h, x2 = 0, y2 = 0;
	float ar = w / float(h);
	// Get zoom box corners
	for (Actors::const_iterator it = actors.begin(); it != actors.end(); ++it) {
		if (!it->is_dead()) {
			b2Vec2 itpos = it->getBody()->GetPosition();
			if (itpos.x < x1) x1 = itpos.x;
			if (itpos.x > x2) x2 = itpos.x;
			if (itpos.y < y1) y1 = itpos.y;
			if (itpos.y > y2) y2 = itpos.y;
		}
	}
	// Add borders and clamp box to screen size
	x1 = clamp(x1 - xmargin, 0.0f, float(w));
	x2 = clamp(x2 + xmargin, 0.0f, float(w));
	y1 = clamp(y1 - ymargin, 0.0f, float(h));
	y2 = clamp(y2 + ymargin, 0.0f, float(h));
	// Correct aspect ratio
	float boxw = (x2-x1), boxh = (y2-y1);
	if (boxh > boxw / ar) boxw = boxh * ar;
	else boxh = boxw / ar;
	float midx = (x1+x2)*0.5f;
	float midy = (y1+y2)*0.5f;
	x1 = midx-boxw*0.5f;
	x2 = midx+boxw*0.5f;
	y1 = midy-boxh*0.5f;
	y2 = midy+boxh*0.5f;
	// Move back inside screen
	float xcorr = 0, ycorr = 0;
	if (x1 < 0) xcorr = -x1;
	if (x2 > w) xcorr = w-x2;
	if (y1 < 0) ycorr = -y1;
	if (y2 > h) ycorr = h-y2;
	x1 += xcorr; x2 += xcorr;
	y1 += ycorr; y2 += ycorr;
	// Do the magic
	glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(x1, x2, y2, y1);
	glMatrixMode(GL_MODELVIEW);

	{ // Background
		static const int texsize = 256;
		CoordArray v_arr, t_arr;
		for (int j = 0; j < h/texsize + 1; j++) {
			for (int i = 0; i < w/texsize + 1; i++) {
				float xx = i * texsize;
				float yy = j * texsize;
				float verts[] = { xx, yy + texsize,
								  xx, yy,
								  xx + texsize, yy,
								  xx + texsize, yy + texsize };
				v_arr.insert(v_arr.end(), &verts[0], &verts[8]);
				t_arr.insert(t_arr.end(), &tex_square[0], &tex_square[8]);
			}
		}
		drawVertexArray(&v_arr[0], &t_arr[0], v_arr.size()/2, texture_background);
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
		it->draw();
	}
	// Players
	for (Actors::const_iterator it = actors.begin(); it != actors.end(); ++it) {
		if (!it->is_dead() && !it->invisible) it->draw();
	}
	// Power-ups
	for (Powerups::const_iterator it = powerups.begin(); it != powerups.end(); ++it) {
		it->draw();
	}
	{ // Water
		CoordArray v_arr, t_arr;
		for (int i = 0; i < w / water_height + 1; i++) {
			float xx = i * water_height;
			float yy = h - water_height;
			float verts[] = { xx, yy + water_height,
							  xx, yy,
							  xx + water_height, yy,
							  xx + water_height, yy + water_height };
			v_arr.insert(v_arr.end(), &verts[0], &verts[8]);
			t_arr.insert(t_arr.end(), &tex_square[0], &tex_square[8]);
		}
		drawVertexArray(&v_arr[0], &t_arr[0], v_arr.size()/2, texture_water);
	}
}
