#include "powerups.hh"
#include "player.hh"


void Powerup::equip(Actor* owner) {
	lifetime = Countdown(3600);
}


void Powerup::unequip(Actor* owner) {

}


void Powerup::touch(Actor* owner, Actor* other) {

}


void Powerup::action(Actor* owner) {
	if (lifetime()) return;
}
