#include <iostream>
#include <Box2D.h>

#include "powerups.hh"
#include "player.hh"

const Powerup::Type Powerup::PowerupTypes[] =
	{ DEATH, INVISIBILITY, MINE, DOUBLEJUMP, PUNCH, GUN };

void Powerup::equip(Actor* owner) {
	if (type == DEATH) { owner->die(); return; }
	// Put large defaults to avoid expiration
	float time = 3600;
	ammo = 10000;
	if (type == INVISIBILITY) { time = 10; owner->invisible = true; }
	else if (type == PUNCH) { time = 10; }
	else if (type == MINE) { ammo = 1; }
	else if (type == GUN) { ammo = 3; }
	else if (type == DOUBLEJUMP) { owner->doublejump = DJUMP_ALLOW; }
	lifetime = Countdown(time);
}


void Powerup::unequip(Actor* owner) {
	if (type == INVISIBILITY) { owner->invisible = false; }
	else if (type == DOUBLEJUMP) { owner->doublejump = DJUMP_DISALLOW; }
}


void Powerup::touch(Actor* owner, Actor* other) {
	if (!owner || !other) return;
	if (type == PUNCH) {
		b2Vec2 v = other->getBody()->GetWorldCenter() - owner->getBody()->GetWorldCenter();
		v.Normalize();
		other->getBody()->ApplyLinearImpulse(1000.0f * v, other->getBody()->GetWorldCenter());
	}
}


void Powerup::action(Actor* owner) {
	if (lifetime()) return;
	if (ammo <= 0) return;
	if (type == MINE) {
		std::cout << "MINE PLACED" << std::endl;
		ammo--;
	} else if (type == GUN) {
		std::cout << "SHOOTING MINIGUN" << std::endl;
		ammo--;
	}
}
