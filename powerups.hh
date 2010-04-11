#pragma once

#include <vector>
#include <Box2D.h>
#include <GL/gl.h>

struct Powerup {
	enum Type { DEATH, INVISIBILITY, MINE, DOUBLEJUMP, PUNCH, MINIGUN, POWERUPS } type;
	Powerup(Type type, GLuint tex): type(type), size(16), texture(tex)
	{ }

	void equip() {}
	void unequip() {}
	void action() {}
	void draw() const {
		float x = getX(), y = getY();
		float vc[] = { x-size, y+size,
		               x-size, y-size,
		               x+size, y-size,
		               x+size, y+size };

		drawVertexArray(&vc[0], getTileTexCoords(type, 4, 4), 4, texture);
	}

	float32 getX() const { return body->GetPosition().x; }
	float32 getY() const { return body->GetPosition().y; }
	float getSize() { return size; }
	b2Body* getBody() { return body; }

	b2Body* body;

	int ammo;
	float lifetime;

	float size;
	GLuint texture;
};


typedef std::vector<Powerup> Powerups;
static Powerup::Type PowerupTypes[] = { Powerup::DEATH, Powerup::INVISIBILITY, Powerup::MINE, Powerup::DOUBLEJUMP, Powerup::PUNCH, Powerup::MINIGUN };
