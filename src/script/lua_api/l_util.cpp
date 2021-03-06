/*
Minetest
Copyright (C) 2013 celeron55, Perttu Ahola <celeron55@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "lua_api/l_util.h"
#include "lua_api/l_internal.h"
#include "common/c_converter.h"
#include "common/c_content.h"
#include "debug.h"
#include "log.h"
#include "tool.h"
#include "settings.h"
#include "main.h"  //required for g_settings, g_settings_path

// debug(...)
// Writes a line to dstream
int ModApiUtil::l_debug(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	// Handle multiple parameters to behave like standard lua print()
	int n = lua_gettop(L);
	lua_getglobal(L, "tostring");
	for (int i = 1; i <= n; i++) {
		/*
			Call tostring(i-th argument).
			This is what print() does, and it behaves a bit
			differently from directly calling lua_tostring.
		*/
		lua_pushvalue(L, -1);  /* function to be called */
		lua_pushvalue(L, i);   /* value to print */
		lua_call(L, 1, 1);
		const char *s = lua_tostring(L, -1);
		if (i>1)
			dstream << "\t";
		if (s)
			dstream << s;
		lua_pop(L, 1);
	}
	dstream << std::endl;
	return 0;
}

// log([level,] text)
// Writes a line to the logger.
// The one-argument version logs to infostream.
// The two-argument version accept a log level: error, action, info, or verbose.
int ModApiUtil::l_log(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	std::string text;
	LogMessageLevel level = LMT_INFO;
	if (lua_isnone(L, 2)) {
		text = lua_tostring(L, 1);
	}
	else {
		std::string levelname = luaL_checkstring(L, 1);
		text = luaL_checkstring(L, 2);
		if(levelname == "error")
			level = LMT_ERROR;
		else if(levelname == "action")
			level = LMT_ACTION;
		else if(levelname == "verbose")
			level = LMT_VERBOSE;
	}
	log_printline(level, text);
	return 0;
}

// setting_set(name, value)
int ModApiUtil::l_setting_set(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	const char *name = luaL_checkstring(L, 1);
	const char *value = luaL_checkstring(L, 2);
	g_settings->set(name, value);
	return 0;
}

// setting_get(name)
int ModApiUtil::l_setting_get(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	const char *name = luaL_checkstring(L, 1);
	try{
		std::string value = g_settings->get(name);
		lua_pushstring(L, value.c_str());
	} catch(SettingNotFoundException &e){
		lua_pushnil(L);
	}
	return 1;
}

// setting_setbool(name)
int ModApiUtil::l_setting_setbool(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	const char *name = luaL_checkstring(L, 1);
	bool value = lua_toboolean(L, 2);
	g_settings->setBool(name, value);
	return 0;
}

// setting_getbool(name)
int ModApiUtil::l_setting_getbool(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	const char *name = luaL_checkstring(L, 1);
	try{
		bool value = g_settings->getBool(name);
		lua_pushboolean(L, value);
	} catch(SettingNotFoundException &e){
		lua_pushnil(L);
	}
	return 1;
}

// setting_save()
int ModApiUtil::l_setting_save(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	if(g_settings_path != "")
		g_settings->updateConfigFile(g_settings_path.c_str());
	return 0;
}

// get_dig_params(groups, tool_capabilities[, time_from_last_punch])
int ModApiUtil::l_get_dig_params(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	std::map<std::string, int> groups;
	read_groups(L, 1, groups);
	ToolCapabilities tp = read_tool_capabilities(L, 2);
	if(lua_isnoneornil(L, 3))
		push_dig_params(L, getDigParams(groups, &tp));
	else
		push_dig_params(L, getDigParams(groups, &tp,
					luaL_checknumber(L, 3)));
	return 1;
}

// get_hit_params(groups, tool_capabilities[, time_from_last_punch])
int ModApiUtil::l_get_hit_params(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	std::map<std::string, int> groups;
	read_groups(L, 1, groups);
	ToolCapabilities tp = read_tool_capabilities(L, 2);
	if(lua_isnoneornil(L, 3))
		push_hit_params(L, getHitParams(groups, &tp));
	else
		push_hit_params(L, getHitParams(groups, &tp,
					luaL_checknumber(L, 3)));
	return 1;
}

// get_password_hash(name, raw_password)
int ModApiUtil::l_get_password_hash(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	std::string name = luaL_checkstring(L, 1);
	std::string raw_password = luaL_checkstring(L, 2);
	std::string hash = translatePassword(name,
			narrow_to_wide(raw_password));
	lua_pushstring(L, hash.c_str());
	return 1;
}

void ModApiUtil::Initialize(lua_State *L, int top)
{
	API_FCT(debug);
	API_FCT(log);

	API_FCT(setting_set);
	API_FCT(setting_get);
	API_FCT(setting_setbool);
	API_FCT(setting_getbool);
	API_FCT(setting_save);

	API_FCT(get_dig_params);
	API_FCT(get_hit_params);

	API_FCT(get_password_hash);
}

