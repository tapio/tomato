#pragma once

#include <Box2D.h>
#include <GL/gl.h>

#include "texture.hh"

class World;

struct SerializedEntity {
	float x, y, vx, vy, a, va;
	char id, type;
	SerializedEntity(float x, float y, float vx, float vy, float a, float va):
		x(x), y(y), vx(vx), vy(vy), a(a), va(va), id(0), type(0) {}
	operator char*() { return reinterpret_cast<char*>(this); } /// overload char cast
	operator char const*() const { return reinterpret_cast<char const*>(this); } /// overload char const cast
};


struct Entity {
	Entity(float size, GLuint tex): world(NULL), body(NULL), size(size), texture(tex)
	{ }

	virtual void draw(int frame = 0, int tiles = 4, bool flipped = false) const {
		float x = getX(), y = getY();
		float vc[] = { x-size, y+size,
		               x-size, y-size,
		               x+size, y-size,
		               x+size, y+size };

		drawVertexArray(&vc[0], getTileTexCoords(frame, tiles, tiles, flipped), 4, texture);
	}

	virtual SerializedEntity serialize() const {
		b2Body* b = getBody();
		b2Vec2 pos = b->GetPosition();
		b2Vec2 vel = b->GetLinearVelocity();
		return SerializedEntity(pos.x, pos.y, vel.x, vel.y, b->GetAngle(), b->GetAngularVelocity());
	}

	virtual void unserialize(std::string data) {
		SerializedEntity* se;
		se = reinterpret_cast<SerializedEntity*>(&data[0]);
		b2Body* b = getBody();
		b->SetTransform(b2Vec2(se->x, se->y), se->a);
		b->SetLinearVelocity(b2Vec2(se->vx, se->vy));
		b->SetAngularVelocity(se->va);
	}

	float32 getX() const { return body->GetPosition().x; }
	float32 getY() const { return body->GetPosition().y; }
	float getSize() const { return size; }
	World* getWorld() const { return world; }
	b2Body* getBody() const { return body; }

	World* world;
	b2Body* body;
	float size;
	GLuint texture;
};
