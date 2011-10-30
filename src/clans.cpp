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
	setClan(id,name,false);
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
	m_names.erase( clanNameNoEx(id) );
	m_ids.erase(id);
}
	
bool ClansManager::clanExists(u16 id) const
{
	return m_ids.find(id) != m_ids.end();
}

bool ClansManager::clanExists(const std::string& name) const
{
	return m_names.find(name) != m_names.end();
}

bool ClansManager::clanDeleted(u16 id) const
{
	return m_deleted.find(id) != m_deleted.end();
}

u16 ClansManager::clanId(const std::string& name) const
{
	std::map<std::string,Clan*>::const_iterator it = m_names.find(name);
	if(it==m_names.end()) return 0;
	return it->second->id;
}

const std::string& ClansManager::clanName(u16 id) const
{
	std::map<u16,Clan>::const_iterator it = m_ids.find(id);
	if(it==m_ids.end()) throw BaseException("Clan doesn't exist");
	return it->second.name;
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

const std::map<u16,Clan>& ClansManager::getClans() const
{
	return m_ids;
}

const std::set<u16>& ClansManager::getDeleted() const
{
	return m_deleted;
}

void ClansManager::save(Settings& args) const
{
	args.setS32("clans-maxId",m_maxId);
	
	std::ostringstream os;
	for(std::map<u16,Clan>::const_iterator it=m_ids.begin(); it!=m_ids.end(); it++){
		os << it->first << ' ' << it->second.name << ' ';
	}
	args.set("clans-names",os.str());

	std::ostringstream os2;
	for(std::map<u16,Clan>::const_iterator it=m_ids.begin(); it!=m_ids.end(); it++){
		if(!it->second.hasSpawnPoint) continue;
		os2 << it->first << ' ' 
			<< it->second.spawnPoint.X << ' ' 
			<< it->second.spawnPoint.Y << ' ' 
			<< it->second.spawnPoint.Z << ' ';
	}
	args.set("clans-spawn",os2.str());

	std::ostringstream os3;
	for(std::set<u16>::const_iterator it=m_deleted.begin(); it!=m_deleted.end(); it++){
		os3 << *it << ' ';
	}
	args.set("clans-deleted",os3.str());
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
			setClan(id,name,false);
		}

		if(args.exists("clans-spawn")){
			s = args.get("clans-spawn");
			std::istringstream is3(s);
			while(is3.good() && !is3.eof()){
				u16 id;
				v3f spawn;
				is3 >> id >> spawn.X >> spawn.Y >> spawn.Z;
				if(id == 0) continue;
				Clan* clan = getClan(id);
				if(!clan) continue;
				clan->hasSpawnPoint = true;
				clan->spawnPoint = spawn;
			}
		}

		s = args.get("clans-deleted");
		std::istringstream is2(s);
		while(is2.good() && !is2.eof()){
			u16 id;
			is2 >> id;
			if(id == 0 ) continue;
			deleteClan(id);
			m_deleted.insert(id);
		}

	}catch(...){
		//TODO: what to do?
		m_maxId = 0;
		m_names.clear();
		m_ids.clear();
		m_deleted.clear();
	}
}

Clan* ClansManager::getClan(u16 id)
{
	std::map<u16,Clan>::iterator it = m_ids.find(id);
	if(it==m_ids.end()) return NULL;
	return &it->second;
}

const Clan* ClansManager::getClan(u16 id) const
{
	std::map<u16,Clan>::const_iterator it = m_ids.find(id);
	if(it==m_ids.end()) return NULL;
	return &it->second;
}

void ClansManager::setClan(u16 id, const std::string& name, bool hasSpawnPoint, v3f spawnPoint)
{
	Clan& clan = m_ids[id];
	clan.name = name;
	clan.id = id;
	clan.hasSpawnPoint = hasSpawnPoint;
	clan.spawnPoint = spawnPoint;
	m_names[name] = &clan;
}

void ClansManager::addDeleted(u16 id)
{
	m_deleted.insert(id);
}

