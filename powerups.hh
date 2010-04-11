#pragma once

#include <vector>
#include <Box2D.h>
#include <GL/gl.h>

#include "util.hh"
#include "entity.hh"

class Actor;

struct Powerup {
	enum Type { DEATH, INVISIBILITY, MINE, DOUBLEJUMP, PUNCH, MINIGUN, POWERUPS, NONE } type;
	const static Type PowerupTypes[];
	static Type Random() { return PowerupTypes[randint(POWERUPS)]; }

	Powerup(Type type = NONE): type(type) { }
	void equip(Actor* owner);
	void unequip(Actor* owner);
	void touch(Actor* owner, Actor* other);
	void action(Actor* owner);

	int ammo;
	Countdown lifetime;
};

struct PowerupEntity: public Entity {

	PowerupEntity(Powerup::Type type, GLuint tex): Entity(16, tex), effect(type)
	{ }

	virtual void draw() const { Entity::draw(effect.type); }

	Powerup effect;
};


typedef std::vector<PowerupEntity> Powerups;
