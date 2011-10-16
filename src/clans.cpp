/*
Minetest-c55
Copyright (C) 2011 celeron55, Perttu Ahola <celeron55@gmail.com>

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

Author: Jan Cychnerski
*/

#include "clans.h"
#include "player.h"
#include "settings.h"

ClansManager::ClansManager():
m_maxId(0)
{}

u16 ClansManager::newClan(const std::string& name, Player* player)
{
	if(	   name.length()==0
		|| name.length()>PLAYERNAME_SIZE
		|| !string_allowed(name, PLAYERNAME_ALLOWED_CHARS)
	  ) return 0;
	if(clanExists(name)) return 0;
	if(m_maxId == 0xFFFF) return 0; //j: szukanie jakiegos wolnego?
	u16 id = ++m_maxId;
	m_idName[id] = name;
	m_nameId[name] = id;
	if(player){
		player->clanOwner = id;
		player->clans.insert(id);
	}
	return id;
}

void ClansManager::deleteClan(u16 id)
{
	if(!clanExists(id))return;
	m_deleted.insert(id);
	m_nameId.erase( clanNameNoEx(id) );
	m_idName.erase(id);
}
	
bool ClansManager::clanExists(u16 id) const
{
	return m_idName.find(id) != m_idName.end();
}

bool ClansManager::clanExists(const std::string& name) const
{
	return m_nameId.find(name) != m_nameId.end();
}

bool ClansManager::clanDeleted(u16 id) const
{
	return m_deleted.find(id) != m_deleted.end();
}

u16 ClansManager::clanId(const std::string& name) const
{
	std::map<std::string,u16>::const_iterator it = m_nameId.find(name);
	if(it==m_nameId.end()) return 0;
	return it->second;
}

const std::string& ClansManager::clanName(u16 id) const
{
	std::map<u16,std::string>::const_iterator it = m_idName.find(id);
	if(it==m_idName.end()) throw BaseException("Clan doesn't exist");
	return it->second;
}

const std::string& ClansManager::clanNameNoEx(u16 id) const
{
	try{
		return clanName(id);
	}catch(std::exception& e){
		static const std::string nullstr("?");
		return nullstr;
	}
}

const std::map<u16,std::string>& ClansManager::getNames() const
{
	return m_idName;
}

void ClansManager::setClan(u16 id, const std::string& name)
{
	m_idName[id] = name;
	m_nameId[name] = id;
}


void ClansManager::save(Settings& args) const
{
	args.setS32("clans-maxId",m_maxId);
	
	std::ostringstream os;
	for(std::map<u16,std::string>::const_iterator it=m_idName.begin(); it!=m_idName.end(); it++){
		os << it->first << ' ' << it->second << ' ';
	}
	args.set("clans-names",os.str());

	std::ostringstream os2;
	for(std::set<u16>::const_iterator it=m_deleted.begin(); it!=m_deleted.end(); it++){
		os2 << *it << ' ';
	}
	args.set("clans-deleted",os2.str());
}

void ClansManager::load(Settings& args)
{
	try{
		m_maxId = args.getS32("clans-maxId");
		
		std::string s = args.get("clans-names");
		std::istringstream is(s);
		while(is.good() && !is.eof()){
			u16 id;
			std::string name;
			is >> id >> name;
			if(id == 0 || name.length() == 0) continue;
			m_idName[id] = name;
			m_nameId[name] = id;
		}

		s = args.get("clans-deleted");
		std::istringstream is2(s);
		while(is2.good() && !is2.eof()){
			u16 id;
			is2 >> id;
			if(id == 0 ) continue;
			m_deleted.insert(id);
		}

	}catch(...){
		//TODO: what to do?
		m_maxId = 0;
		m_idName.clear();
		m_nameId.clear();
		m_deleted.clear();
	}
}

