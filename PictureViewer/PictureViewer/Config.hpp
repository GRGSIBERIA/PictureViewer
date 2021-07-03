#pragma once
#include <Siv3D.hpp>

namespace conf
{
	struct Config
	{
		String current_path;
	};

	INISection get_section(INIData& config, const String& section_name)
	{
		if (!config.hasSection(section_name))
		{
			config.addSection(section_name);
		}
		return config.getSection(section_name);
	}

	String get_value(INIData& config, const String& section, const String& name, const String& default_value)
	{
		const String accessor = section + U"." + name;
		if (!config.hasValue(section, name))
		{
			config[accessor] = default_value;
		}
		return config.get<String>(accessor);
	}

	Config initialize()
	{
		INIData ini;
		if (!ini.load(U"./config.ini"))
		{
			ini.save(U"./config.ini");
		}

		Config config;

		auto sec_path = get_section(ini, U"Path");

		config.current_path = get_value(ini, U"Path", U"CurrentPath", U"./");

		return config;
	}
}
