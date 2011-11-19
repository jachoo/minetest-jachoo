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

#include "teleports.h"
#include "settings.h"

bool TeleportsManager::addNoEx(const std::string& name, const v3s16& coords)
{
	JLOG("Probuje dodac nazwe " << name << " na " << JV3(coords) );
	
	if(coords.X == TELEPORT_IGNORE) return false;

	TeleportLink& link = m_links[name];
	
	int i;
	for(i=0;link.coords[i].X != TELEPORT_IGNORE;i++); //find first empty slot for coords
	if(i>=2) return false;

	for(int j=0;j<2;j++) //if already exists -> report success
		if(link.coords[j] == coords) return true;

	JLOG("Dodaje nazwe " << name << " na " << JV3(coords) );

	link.coords[i] = coords;
	return true;
}

bool TeleportsManager::removeNoEx(const std::string& name, const v3s16& coords)
{
	JLOG("Usuwam " << name << " z " << JV3(coords) << "...");
	links_t::iterator it = m_links.find(name);
	if(it==m_links.end()) return false;

	TeleportLink& link = it->second;

	if(link.coords[0] == coords){
		if(link.coords[1].X == TELEPORT_IGNORE){
			//remove entire link
			m_links.erase(name);
			return true;
		}
		link.coords[0] = link.coords[1];
		link.coords[1].X = TELEPORT_IGNORE;
	}else if(link.coords[1] == coords){
		link.coords[1].X = TELEPORT_IGNORE;
	}else if(link.coords[0].X == TELEPORT_IGNORE && link.coords[1].X == TELEPORT_IGNORE){
		m_links.erase(name);
	}else return false;

	return true;
}

bool TeleportsManager::renameNoEx(const std::string& oldName, const v3s16& coords, const std::string& newName)
{
	JLOG("Zmieniam nazwe " << oldName << " na " << newName);
	removeNoEx(oldName,coords);
	if(!addNoEx(newName,coords)) return false;
	return true;
}

const v3s16& TeleportsManager::getTarget(const std::string& name, const v3s16& not_this) const
{
	JLOG("Odczytuje cel " << name);
	links_t::const_iterator it = m_links.find(name);
	if(it==m_links.end()) throw TeleportManagerException();

	const TeleportLink& link = it->second;

	for(int i=0; i<2; i++)
		if( link.coords[i].X != TELEPORT_IGNORE && link.coords[i] != not_this)
			return link.coords[i];

	throw TeleportManagerException();
}

const v3s16& TeleportsManager::getTarget(const std::string& name) const
{
	static const v3s16 nowhere(-32768,-32768,-32768);
	return getTarget(name,nowhere);
}


void TeleportsManager::save(Settings& args) const
{
	std::ostringstream os;
	for(links_t::const_iterator it=m_links.begin(); it!=m_links.end(); it++){
		const TeleportLink& t = it->second;
		std::string name = it->first;
		str_replace_char(name,' ','\xFF');
		os << name << ' '; //name
		for(int i=0; i<2; i++)  //coords
			if(t.coords[i].X == TELEPORT_IGNORE) //if coord ignored - don't save Y and Z
				os << TELEPORT_IGNORE << ' ';
			else os << t.coords[i].X << ' ' << t.coords[i].Y << ' ' << t.coords[i].Z << ' ';
	}
	args.set("teleports",os.str());
}

void TeleportsManager::load(Settings& args)
{
	try{
		std::string s = args.get("teleports");
		std::istringstream is(s);
		while(is.good() && !is.eof()){
			std::string name;
			is >> name;
			if(is.bad() || is.eof()) break;
			if(name.length() == 0) continue;
			str_replace_char(name,'\xFF',' ');
			for(int i=0; i<2; i++){
				v3s16 v;
				is >> v.X;
				if(v.X != TELEPORT_IGNORE){
					is >> v.Y >> v.Z;
					addNoEx(name,v);
				}
			}
		}
	}catch(...){
		//TODO: what to do?
	}
}



bool getTeleportInfo(TeleportInfo& ti, const std::string& text, bool allowCoords, bool allowUnnamed, bool ignoreDescription)
{
	//possible values:
	//a. X,Y,Z
	//b. X,Y,Z,description
	//c. name
	//d. name,description
	//e. name->targetname
	//f. name->targetname,description
	//g. ->targetname
	//h. ->targetname,description
	//SPACES allowed EVERYWHERE! (but first char maybe)
	//COMMAS allowed in DESCRIPTION
	//(so may be "myhouse->mine,Some very, very, very strange description")
	//a,b correct only if allowCoords=true
	//g,h correct only if allowUnnamed=true

	JLOG("getTeleportInfo(" << text << ")");
#define JLOGTI(x) JLOG( "ti: " << x.thisName << " -> " << x.targetName << " " << JV3(x.targetLocation) << " - " << x.description)

	if(text.empty())return false;

	const int maxCnt = allowCoords ? 4 : 2;
	std::vector<std::string> parts = str_split(text,',',maxCnt);

	//if(parts.size()==0) return false; //impossible?
	if(!allowUnnamed && parts[0].empty()) return false;

	if(allowCoords && parts.size() >= 3){ //possibly coords (a or b)
		try{
			v3s16 tgt;

			tgt.X = core::round_( stof_ex(parts[0]) );
			tgt.Y = core::round_( stof_ex(parts[1]) );
			tgt.Z = core::round_( stof_ex(parts[2]) );

			if(	tgt.X >= MAP_GENERATION_LIMIT || tgt.X <= -MAP_GENERATION_LIMIT ||
				tgt.Y >= MAP_GENERATION_LIMIT || tgt.Y <= -MAP_GENERATION_LIMIT ||
				tgt.Z >= MAP_GENERATION_LIMIT || tgt.Z <= -MAP_GENERATION_LIMIT
			) return false; //bad coords!

			ti.targetLocation = tgt; //good coords

			if(!ignoreDescription && parts.size()==4) //description is given?
				ti.description = parts[3];

			JLOG("Koordynaty: " << JV3(tgt));

			JLOGTI(ti);
			return true;
		}catch(std::ios::failure&){ //bad number format
			JLOG("Bad number format");
		} 
	}

	//find if parts[0] has target name
	int pos = parts[0].find("->");

	if(!allowUnnamed && pos==0) JLOGAND("missing 'thisname'", return false;) //no 'thisname' specified

	if(pos != std::string::npos){ //yes, split it to 'thisname' and 'targetname'
		if(pos>0) ti.thisName = trim(parts[0].substr(0,pos));
		if(pos+2<parts[0].length()) ti.targetName = trim(parts[0].substr(pos+2));
	}else ti.thisName = parts[0]; //no, it's just 'thisname'

	//do we have description?
	if(!ignoreDescription && parts.size()>1){
		//yes, desc is the rest of 'parts'
		for(unsigned i=1; i<parts.size(); i++)
			ti.description += parts[i];
	}

	JLOGTI(ti);
	return true;
}



