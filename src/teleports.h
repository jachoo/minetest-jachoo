/*
Minetest-c55
Copyright (C) 2010-2011 celeron55, Perttu Ahola <celeron55@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/*
	teleports.h - author: Jan Cychnerski
*/

#ifndef TELEPORTS_HEADER
#define TELEPORTS_HEADER

#include "common_irrlicht.h"
#include "utility.h"

#include <map>
#include <vector>
#include <string>

#define TELEPORT_IGNORE -32768

class Settings;

class TeleportManagerException : public BaseException {
public:
	TeleportManagerException():BaseException("Teleport manager exception"){}
};

struct TeleportLink {
	v3s16 coords[2]; //if X == TELEPORT_IGNORE then ignore it!
	//std::string name;

	TeleportLink(){
		coords[0].X = TELEPORT_IGNORE;
		coords[1].X = TELEPORT_IGNORE;
	}
};

class TeleportsManager {
public:

	//NOTE: all methods (except *NoEx) throw if name is not defined!

	//returns false if error
	bool addNoEx(const std::string& name, const v3s16& coords);

	bool removeNoEx(const std::string& name, const v3s16& coords);
	bool renameNoEx(const std::string& oldName, const v3s16& coords, const std::string& newName);

	//returns coords of teleport, but not equal to not_this (if many, returns first)
	const v3s16& getTarget(const std::string& name, const v3s16& not_this) const;

	//returns coords of this name 
	const v3s16& getTarget(const std::string& name) const;

	void save(Settings& args) const;
	void load(Settings& args);

private:
	typedef std::map<std::string,TeleportLink> links_t;
	links_t m_links;
};



#if 1
#define JLOG(x) std::cout << x << std::endl
#define JV3(x) '[' << x.X << ',' << x.Y << ',' << x.Z << ']'
#define JLOGAND(x,y) { JLOG(x); y }
#else
#define JLOG(x)
#define JLOGAND(x,y) y
#endif

struct TeleportInfo {
	std::string thisName, targetName, description;
	v3s16 /*thisLocation,*/ targetLocation;

	TeleportInfo(): targetLocation(TELEPORT_IGNORE,TELEPORT_IGNORE,TELEPORT_IGNORE) {}
};

bool getTeleportInfo(TeleportInfo& ti, const std::string& text, bool allowCoords, bool allowUnnamed, bool ignoreDescription);

#endif

