#pragma once

#include <Box2D.h>
#include <GL/gl.h>

#include "texture.hh"

struct Entity {
	Entity(float size, GLuint tex): body(NULL), size(size), texture(tex)
	{ }

	virtual void draw(int frame = 0, int tiles = 4, bool flipped = false) const {
		float x = getX(), y = getY();
		float vc[] = { x-size, y+size,
		               x-size, y-size,
		               x+size, y-size,
		               x+size, y+size };

		drawVertexArray(&vc[0], getTileTexCoords(frame, tiles, tiles, flipped), 4, texture);
	}

	float32 getX() const { return body->GetPosition().x; }
	float32 getY() const { return body->GetPosition().y; }
	float getSize() const { return size; }
	b2Body* getBody() const { return body; }

	b2Body* body;
	float size;
	GLuint texture;
};
