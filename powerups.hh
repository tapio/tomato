#pragma once

#include <vector>
#include <Box2D.h>
#include <GL/gl.h>

#include "util.hh"
#include "entity.hh"

class Actor;

enum DoubleJumpStatus { DJUMP_DISALLOW, DJUMP_ALLOW, DJUMP_JUMPED };

struct Powerup {
	enum Type { DEATH, INVISIBILITY, MINE, DOUBLEJUMP, PUNCH, GUN, CONFUSION,
	  SUPERBALL, LOGRAV, TELEPORT, POWERUPS, NONE } type;
	const static Type PowerupTypes[];
	static Type Random() { return PowerupTypes[randint(POWERUPS)]; }

	Powerup(Type type = NONE): type(type), ammo(10000) { }
	void equip(Actor* owner);
	void unequip(Actor* owner);
	void touch(Actor* owner, Actor* other);
	void action(Actor* owner);
	bool expired() const { return lifetime() || ammo <= 0; }

	int ammo;
	Countdown lifetime;
};

struct PowerupEntity: public Entity {

	PowerupEntity(Powerup::Type type, GLuint tex): Entity(16, tex), effect(type)
	{ }

	virtual void draw() const { Entity::draw(effect.type); }

	virtual SerializedEntity serialize() const {
		SerializedEntity se = Entity::serialize();
		se.type = effect.type;
		return se;
	}

	virtual void unserialize(std::string data) {
		SerializedEntity* se;
		se = reinterpret_cast<SerializedEntity*>(&data[0]);
		effect.type = Powerup::PowerupTypes[(int)se->type];
		Entity::unserialize(data);
	}


	Powerup effect;
};


typedef std::vector<PowerupEntity> Powerups;
