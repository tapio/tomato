#include <string>
#include <iomanip>
#include <stdexcept>
#include <iostream>
#include <cmath>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include "settings.hh"
#include "filesystem.hh"
#include "util.hh"

int scrW;
int scrH;

bool config_fullscreen;
bool config_zoom;
std::string config_default_gamemode;
int config_default_port;
std::string config_default_host;

namespace fs = boost::filesystem;

Config config;


ConfigItem::ConfigItem(bool bval): m_type("bool"), m_value(bval) { }

ConfigItem::ConfigItem(int ival): m_type("int"), m_value(ival) { }

ConfigItem::ConfigItem(float fval): m_type("float"), m_value(fval) { }

ConfigItem::ConfigItem(std::string sval): m_type("string"), m_value(sval) { }


ConfigItem& ConfigItem::incdec(int dir) {
	if (m_type == "int") {
		int& val = boost::get<int>(m_value);
		int step = boost::get<int>(m_step);
		val = clamp(((val + dir * step)/ step) * step, boost::get<int>(m_min), boost::get<int>(m_max));
	} else if (m_type == "float") {
		double& val = boost::get<double>(m_value);
		double step = boost::get<double>(m_step);
		val = clamp(round((val + dir * step) / step) * step, boost::get<double>(m_min), boost::get<double>(m_max));
	} else if (m_type == "bool") {
		bool& val = boost::get<bool>(m_value);
		val = !val;
	}
	return *this;
}

bool ConfigItem::isDefaultImpl(ConfigItem::Value const& defaultValue) const {
	if (m_type == "bool") return boost::get<bool>(m_value) == boost::get<bool>(defaultValue);
	if (m_type == "int") return boost::get<int>(m_value) == boost::get<int>(defaultValue);
	if (m_type == "float") return boost::get<double>(m_value) == boost::get<double>(defaultValue);
	if (m_type == "string") return boost::get<std::string>(m_value) == boost::get<std::string>(defaultValue);
	if (m_type == "string_list") return boost::get<StringList>(m_value) == boost::get<StringList>(defaultValue);
	throw std::logic_error("ConfigItem::is_default doesn't know type '" + m_type + "'");
}

void ConfigItem::verifyType(std::string const& type) const {
	if (type == m_type) return;
	std::string name = "unknown";
	// Try to find this item in the config map
	for (Config::const_iterator it = config.begin(); it != config.end(); ++it) {
		if (&it->second == this) { name = it->first; break; }
	}
	if (m_type.empty()) throw std::logic_error("Config item " + name + " used in C++ but missing from config schema");
	throw std::logic_error("Config item type mismatch: item=" + name + ", type=" + m_type + ", requested=" + type);
}

int& ConfigItem::i() { verifyType("int"); return boost::get<int>(m_value); }
bool& ConfigItem::b() { verifyType("bool"); return boost::get<bool>(m_value); }
double& ConfigItem::f() { verifyType("float"); return boost::get<double>(m_value); }
std::string& ConfigItem::s() { verifyType("string"); return boost::get<std::string>(m_value); }
ConfigItem::StringList& ConfigItem::sl() { verifyType("string_list"); return boost::get<StringList>(m_value); }

namespace {
	template <typename T, typename VariantAll, typename VariantNum> std::string numericFormat(VariantAll const& value, VariantNum const& multiplier, VariantNum const& step) {
		T m = boost::get<T>(multiplier);
		// Find suitable precision (not very useful for integers, but this code is generic...)
		T s = std::abs(m * boost::get<T>(step));
		unsigned precision = 0;
		while (s > 0.0 && (s *= 10) < 10) ++precision;
		// Format the output
		boost::format fmter("%f");
		fmter % boost::io::group(std::setprecision(precision), double(m) * boost::get<T>(value));
		return fmter.str();
	}

	fs::path origin;  // The primary shared data folder
}

std::string ConfigItem::getValue() const {
	if (m_type == "int") return numericFormat<int>(m_value, m_multiplier, m_step) + m_unit;
	if (m_type == "float") return numericFormat<double>(m_value, m_multiplier, m_step) + m_unit;
	if (m_type == "bool") return boost::get<bool>(m_value) ? "Enabled" : "Disabled";
	if (m_type == "string") return boost::get<std::string>(m_value);
	if (m_type == "string_list") {
		StringList const& sl = boost::get<StringList>(m_value);
		return sl.size() == 1 ? "{" + sl[0] + "}" : (boost::format("%d items") % sl.size()).str();
	}
	throw std::logic_error("ConfigItem::getValue doesn't know type '" + m_type + "'");
}





void readConfig() {
	using boost::property_tree::ptree;
	ptree pt;
	read_ini(getFilePath("data/settings.conf"), pt);

	scrW = pt.get("Settings.screenwidth", 800);
	scrH = pt.get("Settings.screenheight", 600);

	config_fullscreen = pt.get("Settings.fullscreen", false);
	config_zoom = pt.get("Settings.zoom", true);
	config_default_gamemode = pt.get("Settings.gamemode", "classic");
	config_default_host = pt.get("Settings.host", "localhost");
	config_default_port = pt.get("Settings.port", 1234);
}
