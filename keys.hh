#pragma once

#include <string>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <SDL.h>

#include "player.hh"

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
	if (k == "return") return SDLK_RETURN;
	else if (k == "left") return SDLK_LEFT;
	else if (k == "right") return SDLK_RIGHT;
	else if (k == "up") return SDLK_UP;
	else if (k == "down") return SDLK_DOWN;
	else if (k == "space") return SDLK_SPACE;
	else if (k == "backspace") return SDLK_BACKSPACE;
	else if (k == "tab") return SDLK_TAB;
	else if (k == "lalt") return SDLK_LALT;
	else if (k == "ralt") return SDLK_RALT;
	else if (k == "lshift") return SDLK_LSHIFT;
	else if (k == "rshift") return SDLK_RSHIFT;
	else if (k == "lctrl") return SDLK_LCTRL;
	else if (k == "rctrl") return SDLK_RCTRL;
	//else if (k == "") reurn SDLK_;
	// ASCII keys
	else if (k.length() == 1) return k[0];
	return 0;
}

void parse_keys(Players& players, std::string filename = "keys.conf") {
	std::ifstream file(filename.c_str(), std::ios::binary);
	if (!file.is_open()) throw std::runtime_error("Couldn't open key config file "+filename);
	Players::iterator pl = players.begin();
	while (!file.eof() && pl != players.end()) {
		std::string row;
		// Read a line
		getline(file, row); boost::trim(row);
		// Ignore comments and empty lines
		if (row[0] == '#' || row.length() == 0) continue;
		// Next player
		if (row.substr(0,7) == "[Player") { pl = players.begin() + (row[7] - 49); continue; }
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
