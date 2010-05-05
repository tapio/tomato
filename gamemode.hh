#pragma once

#include <string>
#include <stdexcept>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string/case_conv.hpp>

#include "gamemode.hh"
#include "settings.hh"
#include "filesystem.hh"
#include "util.hh"
#include "powerups.hh"

class GameMode {

  public:
	GameMode(std::string gametypefile) {
		using boost::property_tree::ptree;
		ptree pt;
		read_ini(gametypefile, pt);

		name = pt.get("Gametype.name", "Unknown");
		round_time = pt.get("Gametype.timelimit", 0);

		spawn_delay_player = pt.get("Players.respawntime", 0.0f);

		max_powerups = pt.get("Powerups.limit", 4);
		spawn_min_delay_powerup = pt.get("Powerups.mindelay", 5.0f);
		spawn_max_delay_powerup = pt.get("Powerups.maxdelay", 10.0f);
		// Parse allowed powerups
		std::string allowpw = boost::to_lower_copy(pt.get("Powerups.allow", "all"));
		int all_at_once = 0;
		if (allowpw == "" || allowpw == "all") all_at_once = 1;
		else if (allowpw == "none" || allowpw[0] == '0') { all_at_once = -1; max_powerups = 0; }
		// Initialize
		for (int i = 0; i < Powerup::POWERUPS; ++i) powerups[i] = all_at_once > 0 ? true : false;
		if (all_at_once == 0) {
			// Loop through the string
			for (size_t i = 0; i < allowpw.length(); ++i) {
				if (allowpw[i] == ' ' || allowpw[i] == '\t') continue;
				if ((allowpw[i] < '1' || allowpw[i] > '9') && (allowpw[i] < 'a' || allowpw[i] > 'f'))
					throw std::runtime_error("Invalid powerup allowing in game mode definition: " + allowpw);
				unsigned j = hex2num(std::string(1, allowpw[i])) - 1;
				powerups[j] = true;
			}
		}
	}

	/// Resets and starts the round
	void reset() {
		round_timer = Countdown(round_time);
	}

	Powerup::Type randPowerup() {
		int safetyswitch = 20;
		do {
			Powerup::Type putype = Powerup::Random();
			if (powerups[putype]) return putype;
		} while (--safetyswitch > 0);
		return Powerup::NONE;
	}

	std::string getName() const { return name; }
	double timeLeft() const { return round_timer(); }
	float randPowerupDelay() const { return randf(spawn_min_delay_powerup, spawn_max_delay_powerup); }
	int getMaxPowerups() const { return max_powerups; }

  private:

	std::string name;
	float round_time;
	int max_powerups;
	float spawn_min_delay_powerup;
	float spawn_max_delay_powerup;
	float spawn_delay_player;
	bool powerups[Powerup::POWERUPS];

	Countdown round_timer;
};
