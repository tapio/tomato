#include "player.hh"

const std::string Actor::Names[] = { "Fresh", "Frozen", "Raw", "Roasted" };

void Actor::brains() {
	static Countdown actiontimer(1.0);
	static bool stopped = false;
	if (actiontimer()) {
		stopped = false;
		if (randbool() && randbool() && randbool()) stopped = true;
		if (ladder != LADDER_NO && randbool()) jump();
		else move(randbool() ? -1 : 1);
		actiontimer = Countdown(randf(0.5f, 1.5f));
	} else if (!stopped) {
		if (ladder != LADDER_NO && randbool()) jump();
		else move(dir);
	}
}

void Actor::move(int direction) {
	if (reversecontrols) direction = -direction;
	if (direction != dir) { dir = direction; return; }
	if (direction == dir || can_jump()) {
		// Calc base speed depending on state
		float speed = airborne ? speed_move_airborne : speed_move_ground;
		if (ladder == LADDER_CLIMBING) speed = speed_move_ladder;
		// Apply direction
		speed *= direction;
		// Get old speed
		b2Vec2 v = body->GetLinearVelocity();
		// Determine if jumping direction is already chosen
		if (jump_dir == 0 && airborne && ladder == LADDER_NO && std::abs(v.x) > 0.01f) jump_dir = sign(v.x);
		else if (jump_dir != 0 && airborne && ladder == LADDER_NO && sign(v.x) != sign(jump_dir))
			body->SetLinearVelocity(b2Vec2(0, v.y));
		else if (!airborne || ladder != LADDER_NO) jump_dir = 0;
		// If airborne, only slow down the existing speed if trying to turn
		if (airborne && jump_dir != 0 && direction != jump_dir) {
			body->ApplyForce(b2Vec2(direction * 3, 0), body->GetWorldCenter());
		} else {
			// Don't kill existing higher velocity
			if (direction == dir && std::abs(v.x) > std::abs(speed)) speed = v.x;
			// Set the speed
			//body->SetLinearVelocity(b2Vec2(speed, v.y));
			if (std::abs(v.x) < std::abs(speed)) body->ApplyForce(b2Vec2(speed * 5, 0), body->GetWorldCenter());
		}
	}
	if (airborne) anim_frame = 0;
	else anim_frame = int(GetSecs()*15) % 4;
	dir = direction;
}

void Actor::jump(bool forcejump) {
	if (ladder != LADDER_NO) {
		ladder = LADDER_CLIMBING;
		body->SetLinearVelocity(b2Vec2(0.0f, -speed_climb));
		return;
	}
	if (!can_jump() && !forcejump) return;
	body->SetLinearVelocity(b2Vec2(body->GetLinearVelocity().x, -speed_jump));
	// Handle double jump
	if (airborne && jumping == 0 && doublejump == DJUMP_ALLOW)
		doublejump = DJUMP_JUMPED;
	jumping++;
}

void Actor::duck() {
	if (ladder == LADDER_YES || ladder == LADDER_CLIMBING) {
		ladder = LADDER_CLIMBING;
		body->SetLinearVelocity(b2Vec2(0.0f, speed_climb));
		return;
	}
}

void Actor::stop() {
	if (!airborne || ladder == LADDER_CLIMBING) {
		body->SetLinearVelocity(b2Vec2(0.0f, body->GetLinearVelocity().y));
	}
	anim_frame = 0;
}

void Actor::end_jumping() {
	jumping = 0;
	if (ladder == Actor::LADDER_CLIMBING) {
		body->SetLinearVelocity(b2Vec2(body->GetLinearVelocity().x, 0.0f));
	}
}
