#include <iostream>
#include <Box2D.h>

#include "powerups.hh"
#include "player.hh"
#include "world.hh"

const Powerup::Type Powerup::PowerupTypes[] =
	{ DEATH, INVISIBILITY, MINE, DOUBLEJUMP, PUNCH, GUN, CONFUSION, SUPERBALL, LOGRAV };

void Powerup::equip(Actor* owner) {
	if (type == DEATH) { owner->die(); return; }
	// Put large defaults to avoid expiration
	float time = 3600;
	ammo = 10000;
	if (type == INVISIBILITY) { time = 10; owner->invisible = true; }
	else if (type == CONFUSION) { time = 10; owner->reversecontrols = true; }
	else if (type == PUNCH) { time = 20; }
	else if (type == MINE) { ammo = 1; }
	else if (type == GUN) { ammo = 3; }
	else if (type == DOUBLEJUMP) { owner->doublejump = DJUMP_ALLOW; }
	else if (type == SUPERBALL) { time = 10; owner->getBody()->GetFixtureList()->SetRestitution(1.5f); }
	else if (type == LOGRAV) { time = 10; owner->lograv = true; }
	lifetime = Countdown(time);
}


void Powerup::unequip(Actor* owner) {
	if (type == INVISIBILITY) owner->invisible = false;
	else if (type == CONFUSION) owner->reversecontrols = false;
	else if (type == DOUBLEJUMP) owner->doublejump = DJUMP_DISALLOW;
	else if (type == SUPERBALL) owner->getBody()->GetFixtureList()->SetRestitution(PLAYER_RESTITUTION);
	else if (type == LOGRAV) owner->lograv = false;
}


void Powerup::touch(Actor* owner, Actor* other) {
	if (!owner || !other) return;
	if (type == PUNCH) {
		std::cout << "PUNCH" << std::endl;
		b2Vec2 v = other->getBody()->GetWorldCenter() - owner->getBody()->GetWorldCenter();
		v.Normalize();
		other->getBody()->ApplyLinearImpulse(50000.0f * v, other->getBody()->GetWorldCenter());
	}
}


void Powerup::action(Actor* owner) {
	if (lifetime()) return;
	if (ammo <= 0) return;
	if (type == MINE) {
		b2Vec2 pos = owner->getBody()->GetWorldCenter();
		owner->getWorld()->addMine(pos.x + owner->dir * owner->getSize() * 1.8, pos.y);
		ammo--;
	} else if (type == GUN) {
		Actor* target = owner->getWorld()->shoot(*owner);
		if (target) target->die();
		ammo--;
	}
}
