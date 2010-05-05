#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "settings.hh"
#include "filesystem.hh"

int scrW;
int scrH;

bool config_fullscreen;
bool config_zoom;
int config_default_port;
std::string config_default_host;


void readConfig() {
	using boost::property_tree::ptree;
	ptree pt;
	read_ini(getFilePath("data/settings.conf"), pt);

	scrW = pt.get("Settings.screenwidth", 800);
	scrH = pt.get("Settings.screenheight", 600);

	config_fullscreen = pt.get("Settings.fullscreen", false);
	config_zoom = pt.get("Settings.zoom", true);
	config_default_host = pt.get("Settings.host", "localhost");
	config_default_port = pt.get("Settings.port", 1234);
}
