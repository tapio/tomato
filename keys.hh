#pragma once

#include <string>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <SFML/Window.hpp>

#include "player.hh"
#include "filesystem.hh"

namespace {
	/// Returns a word (delimited by delim) in a string st at position pos (1-based)
	std::string getWord(std::string& st, size_t pos, char delim) {
		std::istringstream iss(st);
		std::string ret;
		for (size_t i = 1; i <= pos; i++)
			getline(iss, ret, delim);
		return ret;
	}
}

// TODO
int parse_key(std::string k) {
	if (k == "return") return sf::Key::Return;
	else if (k == "left") return sf::Key::Left;
	else if (k == "right") return sf::Key::Right;
	else if (k == "up") return sf::Key::Up;
	else if (k == "down") return sf::Key::Down;
	else if (k == "space") return sf::Key::Space;
	else if (k == "backspace") return sf::Key::Back;
	else if (k == "tab") return sf::Key::Tab;
	else if (k == "lalt") return sf::Key::LAlt;
	else if (k == "ralt") return sf::Key::RAlt;
	else if (k == "lshift") return sf::Key::LShift;
	else if (k == "rshift") return sf::Key::RShift;
	else if (k == "lctrl") return sf::Key::LControl;
	else if (k == "rctrl") return sf::Key::RControl;
	//else if (k == "") reurn sf::Key::;
	// ASCII keys
	else if (k.length() == 1) return k[0];
	return 0;
}

void parse_keys(Players& players, std::string filename = "keys.conf") {
	std::ifstream file(filename.c_str(), std::ios::binary);
	if (!file.is_open()) throw std::runtime_error("Couldn't open key config file "+filename);
	bool firstheader = true;
	Players::iterator pl = players.begin();
	while (!file.eof() && pl != players.end()) {
		if (pl->type != Actor::HUMAN) { ++pl; continue; }
		std::string row;
		// Read a line
		getline(file, row); boost::trim(row);
		// Ignore comments and empty lines
		if (row[0] == '#' || row.length() == 0) continue;
		// Next player
		if (row.substr(0,7) == "[Player") {
			if (firstheader) firstheader = false;
			else ++pl;
			continue;
		}
		// Parse variable / value
		std::string var = getWord(row, 1, '='); boost::trim(var); boost::to_lower(var);
		std::string val = getWord(row, 2, '='); boost::trim(val); boost::to_lower(val);
		// Interpret
		if (var == "action") pl->KEY_ACTION = parse_key(val);
		else if (var == "left") pl->KEY_LEFT = parse_key(val);
		else if (var == "right") pl->KEY_RIGHT = parse_key(val);
		else if (var == "jump") pl->KEY_UP = parse_key(val);
		else if (var == "duck") pl->KEY_DOWN = parse_key(val);
		else throw std::runtime_error("Error in key config "+filename+": "+row);
	}
}
