#pragma once

#include <boost/variant.hpp>
#include <map>
#include <vector>
#include <string>

extern int scrW;
extern int scrH;

extern bool config_fullscreen;
extern bool config_zoom;

extern std::string config_default_gamemode;

extern int config_default_port;
extern std::string config_default_host;

void readConfig();

/// configuration option
class ConfigItem {
  public:
	typedef std::vector<std::string> StringList; ///< a list of strings
	ConfigItem() {}
	ConfigItem(bool bval);
	ConfigItem(int ival);
	ConfigItem(float fval);
	ConfigItem(std::string sval);
	ConfigItem& operator++() { return incdec(1); } ///< increments config value
	ConfigItem& operator--() { return incdec(-1); } ///< decrements config value
	/// Is the current value the same as the default value (factory setting or system setting)
	bool isDefault(bool factory = false) const { return isDefaultImpl(factory ? m_factoryDefaultValue : m_defaultValue); }
	std::string get_type() const { return m_type; } ///< get the field type
	int& i(); ///< Access integer item
	bool& b(); ///< Access boolean item
	double& f(); ///< Access floating-point item
	std::string& s(); ///< Access string item
	StringList& sl(); ///< Access stringlist item
	void reset(bool factory = false) { m_value = factory ? m_factoryDefaultValue : m_defaultValue; } ///< Reset to default
	void makeSystem() { m_defaultValue = m_value; } ///< Make current value the system default (used when saving system config)
	std::string getValue() const; ///< Get a human-readable representation of the current value
	std::string const& getShortDesc() const { return m_shortDesc; } ///< get the short description for this ConfigItem
	std::string const& getLongDesc() const { return m_longDesc; } ///< get the long description for this ConfigItem

  private:
	void verifyType(std::string const& t) const; ///< throws std::logic_error if t != type
	ConfigItem& incdec(int dir); ///< Increment/decrement by dir steps (must be -1 or 1)
	std::string m_type;
	std::string m_shortDesc;
	std::string m_longDesc;

	typedef boost::variant<bool, int, double, std::string, StringList> Value;
	bool isDefaultImpl(Value const& defaultValue) const;
	Value m_value; ///< The current value
	Value m_factoryDefaultValue; ///< The value from config schema
	Value m_defaultValue; ///< The value from config schema or system config
	boost::variant<int, double> m_step, m_min, m_max;
	boost::variant<int, double> m_multiplier;
	std::string m_unit;
};


typedef std::map<std::string, ConfigItem> Config;
extern Config config; ///< A global variable that contains all config items
