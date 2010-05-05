#pragma once

#include <string>

extern int scrW;
extern int scrH;

extern bool config_fullscreen;
extern bool config_zoom;

extern std::string config_default_gamemode;

extern int config_default_port;
extern std::string config_default_host;

void readConfig();
