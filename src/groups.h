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
	Groups.h - author: jachoo <g.j.g@interia.pl>
*/

#ifndef GROUPS_HEADER
#define GROUPS_HEADER

#include "common_irrlicht.h"
#include "utility.h"
#include "player.h"

#include <map>
#include <set>
#include <string>

class GroupsManager {
public:

	GroupsManager();

	u16 newGroup(const std::string& name, Player* player);
	
	bool groupExists(u16 id) const;
	bool groupExists(const std::string& name) const;

	bool groupDeleted(u16 id) const;

	u16 groupId(const std::string& name) const;
	const std::string& groupName(u16 id) const; //throws if not existing group
	const std::string& groupNameNoEx(u16 id) const;
	
	const std::map<u16,std::string>& getNames() const;
	void setGroup(u16 id, const std::string& name);

	void save(Settings& args) const;
	void load(Settings& args);

	//serializacja - wysylanie

protected:

	std::map<u16,std::string> m_idName;
	std::map<std::string,u16> m_nameId;
	std::set<u16> m_deleted;
	u16 m_maxId;

private:
};

#endif

