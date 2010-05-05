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
#include "player.hh"

class GameMode {

  public:
	GameMode(std::string gametypefile) {
		using boost::property_tree::ptree;
		ptree pt;
		read_ini(gametypefile, pt);
		// Game mode
		name = pt.get("Gamemode.name", "Unknown");
		timelimit = pt.get("Gamemode.timelimit", 0);
		scorelimit = pt.get("Gamemode.scorelimit", 0);
		rounds = pt.get("Gamemode.rounds", 1);
		// Scoring
		points_drowned = pt.get("Scoring.drowned", -1);
		points_killed = pt.get("Scoring.killed", -1);
		points_killer = pt.get("Scoring.killer", 0);
		// Players
		spawn_delay_player = pt.get("Players.respawntime", 0.0f);
		int default_powerup = pt.get("Players.powerup", 0);
		if (default_powerup != 0)
			std::cout << "Warning: Unimplemented game mode modifier 'Players.powerup'." << std::endl;
		// Powerups
		max_powerups = pt.get("Powerups.limit", 4);
		spawn_min_delay_powerup = pt.get("Powerups.mindelay", 5.0f);
		spawn_max_delay_powerup = pt.get("Powerups.maxdelay", 10.0f);
		{ // Parse allowed powerups
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
	}

	/// Start round
	bool startRound() {
		if (rounds > 0) {
			rounds--;
			round_timer = Countdown(timelimit);
			return true;
		} return false;
	}

	/// Has the round ended?
	bool roundEnded() const {
		if (timelimit > 0 && round_timer()) return true;
		else return false;
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
	float timelimit;
	int scorelimit;
	int rounds;
	int points_drowned;
	int points_killed;
	int points_killer;
	float spawn_delay_player;
	int max_powerups;
	float spawn_min_delay_powerup;
	float spawn_max_delay_powerup;
	bool powerups[Powerup::POWERUPS];

	Countdown round_timer;
	Actor* winning;
};
