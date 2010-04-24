#pragma once

#include "config.hh"

std::string inline getFilePath(std::string file) {
	return std::string("../" SHARED_DATA_DIR "/") + file;
}
