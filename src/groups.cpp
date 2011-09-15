#include "groups.h"
#include "player.h"

GroupsManager::GroupsManager():
m_maxId(0)
{}

u16 GroupsManager::newGroup(const std::string& name, Player* player)
{
	if(	   name.length()==0
		|| name.length()>PLAYERNAME_SIZE
		|| !string_allowed(name, PLAYERNAME_ALLOWED_CHARS)
	  ) return 0;
	if(groupExists(name)) return 0;
	if(m_maxId == 0xFFFF) return 0; //j: szukanie jakiegos wolnego?
	u16 id = ++m_maxId;
	m_idName[id] = name;
	m_nameId[name] = id;
	if(player){
		player->groupOwner = id;
		player->groups.insert(id);
	}
	return id;
}
	
bool GroupsManager::groupExists(u16 id) const
{
	return m_idName.find(id) != m_idName.end();
}

bool GroupsManager::groupExists(const std::string& name) const
{
	return m_nameId.find(name) != m_nameId.end();
}

bool GroupsManager::groupDeleted(u16 id) const
{
	return m_deleted.find(id) != m_deleted.end();
}

u16 GroupsManager::groupId(const std::string& name) const
{
	std::map<std::string,u16>::const_iterator it = m_nameId.find(name);
	if(it==m_nameId.end()) return 0;
	return it->second;
}

const std::string& GroupsManager::groupName(u16 id) const
{
	std::map<u16,std::string>::const_iterator it = m_idName.find(id);
	if(it==m_idName.end()) throw std::exception("Group doesn't exist"); //TODO: some other exception
	return it->second;
}

const std::string& GroupsManager::groupNameNoEx(u16 id) const
{
	try{
		return groupName(id);
	}catch(std::exception& e){
		static const std::string nullstr("?");
		return nullstr;
	}
}

const std::map<u16,std::string>& GroupsManager::getNames() const
{
	return m_idName;
}

void GroupsManager::setGroup(u16 id, const std::string& name)
{
	m_idName[id] = name;
	m_nameId[name] = id;
}


void GroupsManager::save(Settings& args) const
{
	args.setS32("groups-maxId",m_maxId);
	
	std::ostringstream os;
	for(std::map<u16,std::string>::const_iterator it=m_idName.begin(); it!=m_idName.end(); it++){
		os << it->first << ' ' << it->second << ' ';
	}
	args.set("groups-names",os.str());

	std::ostringstream os2;
	for(std::set<u16>::const_iterator it=m_deleted.begin(); it!=m_deleted.end(); it++){
		os2 << *it << ' ';
	}
	args.set("groups-deleted",os2.str());
}

void GroupsManager::load(Settings& args)
{
	try{
		m_maxId = args.getS32("groups-maxId");
		
		std::string s = args.get("groups-names");
		std::istringstream is(s);
		while(is.good() && !is.eof()){
			u16 id;
			std::string name;
			is >> id >> name;
			if(id == 0 || name.length() == 0) continue;
			m_idName[id] = name;
			m_nameId[name] = id;
		}

		s = args.get("groups-deleted");
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

