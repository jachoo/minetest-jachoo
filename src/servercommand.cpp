/*
Part of Minetest-c55
Copyright (C) 2010-11 celeron55, Perttu Ahola <celeron55@gmail.com>
Copyright (C) 2011 Ciaran Gultnieks <ciaran@ciarang.com>

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include "servercommand.h"
#include "utility.h"
#include "settings.h"

#define PP(x) "("<<(x).X<<","<<(x).Y<<","<<(x).Z<<")"

void cmd_status(std::wostringstream &os,
	ServerCommandContext *ctx)
{
	os<<ctx->server->getStatusString();
}

void cmd_me(std::wostringstream &os,
	ServerCommandContext *ctx)
{
	std::wstring name = narrow_to_wide(ctx->player->getName());
	os << L"* " << name << L" " << ctx->paramstring;
	ctx->flags |= SEND_TO_OTHERS | SEND_NO_PREFIX;
}

void cmd_privs(std::wostringstream &os,
	ServerCommandContext *ctx)
{
	if(ctx->parms.size() == 1)
	{
		// Show our own real privs, without any adjustments
		// made for admin status
		os<<L"-!- " + narrow_to_wide(privsToString(
				ctx->server->getPlayerAuthPrivs(ctx->player->getName())));
		return;
	}

	if((ctx->privs & PRIV_PRIVS) == 0)
	{
		os<<L"-!- You don't have permission to do that";
		return;
	}
		
	Player *tp = ctx->env->getPlayer(wide_to_narrow(ctx->parms[1]).c_str());
	if(tp == NULL)
	{
		os<<L"-!- No such player";
		return;
	}
	
	os<<L"-!- " + narrow_to_wide(privsToString(ctx->server->getPlayerAuthPrivs(tp->getName())));
}

void cmd_grantrevoke(std::wostringstream &os,
	ServerCommandContext *ctx)
{
	if(ctx->parms.size() != 3)
	{
		os<<L"-!- Missing parameter";
		return;
	}

	if((ctx->privs & PRIV_PRIVS) == 0)
	{
		os<<L"-!- You don't have permission to do that";
		return;
	}

	u64 newprivs = stringToPrivs(wide_to_narrow(ctx->parms[2]));
	if(newprivs == PRIV_INVALID)
	{
		os<<L"-!- Invalid privileges specified";
		return;
	}

	Player *tp = ctx->env->getPlayer(wide_to_narrow(ctx->parms[1]).c_str());
	if(tp == NULL)
	{
		os<<L"-!- No such player";
		return;
	}
	
	std::string playername = wide_to_narrow(ctx->parms[1]);
	u64 privs = ctx->server->getPlayerAuthPrivs(playername);

	if(ctx->parms[0] == L"grant"){
		privs |= newprivs;
		actionstream<<ctx->player->getName()<<" grants "
				<<wide_to_narrow(ctx->parms[2])<<" to "
				<<playername<<std::endl;

		std::wstring msg;
		msg += narrow_to_wide(ctx->player->getName());
		msg += L" granted you the privilege \"";
		msg += ctx->parms[2];
		msg += L"\"";
		ctx->server->notifyPlayer(playername.c_str(), msg);
	} else {
		privs &= ~newprivs;
		actionstream<<ctx->player->getName()<<" revokes "
				<<wide_to_narrow(ctx->parms[2])<<" from "
				<<playername<<std::endl;

		std::wstring msg;
		msg += narrow_to_wide(ctx->player->getName());
		msg += L" revoked from you the privilege \"";
		msg += ctx->parms[2];
		msg += L"\"";
		ctx->server->notifyPlayer(playername.c_str(), msg);
	}
	
	ctx->server->setPlayerAuthPrivs(playername, privs);
	
	os<<L"-!- Privileges change to ";
	os<<narrow_to_wide(privsToString(privs));
}

void cmd_time(std::wostringstream &os,
	ServerCommandContext *ctx)
{
	if(ctx->parms.size() != 2)
	{
		os<<L"-!- Missing parameter";
		return;
	}

	if((ctx->privs & PRIV_SETTIME) ==0)
	{
		os<<L"-!- You don't have permission to do that";
		return;
	}

	u32 time = stoi(wide_to_narrow(ctx->parms[1]));
	ctx->server->setTimeOfDay(time);
	os<<L"-!- time_of_day changed.";

	actionstream<<ctx->player->getName()<<" sets time "
			<<time<<std::endl;
}

void cmd_shutdown(std::wostringstream &os,
	ServerCommandContext *ctx)
{
	if((ctx->privs & PRIV_SERVER) ==0)
	{
		os<<L"-!- You don't have permission to do that";
		return;
	}

	actionstream<<ctx->player->getName()
			<<" shuts down server"<<std::endl;

	ctx->server->requestShutdown();
					
	os<<L"*** Server shutting down (operator request)";
	ctx->flags |= SEND_TO_OTHERS;
}

void cmd_setting(std::wostringstream &os,
	ServerCommandContext *ctx)
{
	if((ctx->privs & PRIV_SERVER) ==0)
	{
		os<<L"-!- You don't have permission to do that";
		return;
	}

	/*std::string confline = wide_to_narrow(
			ctx->parms[1] + L" = " + ctx->params[2]);*/

	std::string confline = wide_to_narrow(ctx->paramstring);
	
	actionstream<<ctx->player->getName()
			<<" sets: "<<confline<<std::endl;

	g_settings->parseConfigLine(confline);
	
	ctx->server->saveConfig();

	os<< L"-!- Setting changed and configuration saved.";
}

void cmd_teleport(std::wostringstream &os,
	ServerCommandContext *ctx)
{
	if((ctx->privs & PRIV_TELEPORT) ==0)
	{
		os<<L"-!- You don't have permission to do that";
		return;
	}

	if(ctx->parms.size() != 2)
	{
		os<<L"-!- Missing parameter";
		return;
	}

	std::vector<std::wstring> coords = str_split(ctx->parms[1], L',');
	if(coords.size() != 3)
	{
		os<<L"-!- You can only specify coordinates currently";
		return;
	}

	v3f dest(stoi(coords[0])*10, stoi(coords[1])*10, stoi(coords[2])*10);

	actionstream<<ctx->player->getName()<<" teleports from "
			<<PP(ctx->player->getPosition()/BS)<<" to "
			<<PP(dest/BS)<<std::endl;

	ctx->player->setPosition(dest);
	ctx->server->SendMovePlayer(ctx->player);

	os<< L"-!- Teleported.";
}

void cmd_banunban(std::wostringstream &os, ServerCommandContext *ctx)
{
	if((ctx->privs & PRIV_BAN) == 0)
	{
		os<<L"-!- You don't have permission to do that";
		return;
	}

	if(ctx->parms.size() < 2)
	{
		std::string desc = ctx->server->getBanDescription("");
		os<<L"-!- Ban list: "<<narrow_to_wide(desc);
		return;
	}
	if(ctx->parms[0] == L"ban")
	{
		Player *player = ctx->env->getPlayer(wide_to_narrow(ctx->parms[1]).c_str());

		if(player == NULL)
		{
			os<<L"-!- No such player";
			return;
		}

		try{
			Address address = ctx->server->getPeerAddress(player->peer_id);
			std::string ip_string = address.serializeString();
		ctx->server->setIpBanned(ip_string, player->getName());
		os<<L"-!- Banned "<<narrow_to_wide(ip_string)<<L"|"
				<<narrow_to_wide(player->getName());

		actionstream<<ctx->player->getName()<<" bans "
				<<player->getName()<<" / "<<ip_string<<std::endl;
		} catch(con::PeerNotFoundException){
			dstream<<__FUNCTION_NAME<<": peer was not found"<<std::endl;
	}
	}
	else
	{
		std::string ip_or_name = wide_to_narrow(ctx->parms[1]);
		std::string desc = ctx->server->getBanDescription(ip_or_name);
		ctx->server->unsetIpBanned(ip_or_name);
		os<<L"-!- Unbanned "<<narrow_to_wide(desc);

		actionstream<<ctx->player->getName()<<" unbans "
				<<ip_or_name<<std::endl;
	}
}

void cmd_clearobjects(std::wostringstream &os,
	ServerCommandContext *ctx)
{
	if((ctx->privs & PRIV_SERVER) ==0)
	{
		os<<L"-!- You don't have permission to do that";
		return;
	}

	actionstream<<ctx->player->getName()
			<<" clears all objects"<<std::endl;
	
	{
		std::wstring msg;
		msg += L"Clearing all objects. This may take long.";
		msg += L" You may experience a timeout. (by ";
		msg += narrow_to_wide(ctx->player->getName());
		msg += L")";
		ctx->server->notifyPlayers(msg);
	}

	ctx->env->clearAllObjects();
					
	actionstream<<"object clearing done"<<std::endl;
	
	os<<L"*** cleared all objects";
	ctx->flags |= SEND_TO_OTHERS;
}


//j
void cmd_clanNew(std::wostringstream &os,
	ServerCommandContext *ctx)
{
	try{
		if((ctx->privs & PRIV_CLANS) == 0)
			throw BaseException("You don't have permission to do that");

		if(ctx->parms.size() != 2)
			throw BaseException("Missing parameter(s) - should be /clan-new clan");

		if(ctx->player->clanOwner)
			throw BaseException("You can define only one clan!");
		//TODO: show actual player's owned clan (so he can delete it and create new)

		std::string clanName = wide_to_narrow(ctx->parms[1]);

		u16 clanId = ctx->env->clansManager.newClan(clanName,ctx->player);

		if(clanId>0){
			ctx->server->BroadcastClanName(clanId,clanName);
			ctx->server->SendPlayerClan(ctx->player,false,clanId);
			os<< L"-!- Clan '"<<ctx->parms[1]<<"' added.";
		}
		else throw BaseException("Clan already exists or other error");

	}catch(BaseException& ex){
		std::wstring msg = narrow_to_wide(std::string(ex.what()));
		os << L"-!- Error - clan NOT added (" << msg << L")";
	}
}

//j
void cmd_clanDelete(std::wostringstream &os,
	ServerCommandContext *ctx)
{
	try{
		if((ctx->privs & PRIV_CLANS) == 0)
			throw BaseException("You don't have permission to do that");

		if(ctx->parms.size() != 2)
			throw BaseException("Missing parameter(s) - should be /clan-delete clan");

		std::string clanName = wide_to_narrow(ctx->parms[1]);
		u16 clanId = ctx->env->clansManager.clanId(clanName);

		if(!clanId)
			throw BaseException("Bad clan name or clan deleted");

		if( (ctx->privs & PRIV_CLANS_ADMIN) == 0 && clanId != ctx->player->clanOwner)
			throw BaseException("Only clan's owner may delete it!");

		ctx->env->clansManager.deleteClan(clanId);
		if(ctx->player->clanOwner == clanId) ctx->player->clanOwner = 0;
		else; //TODO: find clan's real owner and inform reset his clanOwner variable

		ctx->server->BroadcastClanDeleted(clanId);
		os<< L"-!- Clan '"<<ctx->parms[1]<<"' deleted.";

	}catch(BaseException& ex){
		std::wstring msg = narrow_to_wide(std::string(ex.what()));
		os << L"-!- Error - clan not deleted (" << msg << L")";
	}
}

void cmd_clanJoin(std::wostringstream &os,
	ServerCommandContext *ctx)
{
	try{

		if((ctx->privs & PRIV_CLANS) == 0) throw BaseException("You don't have permission to do that");

		if(ctx->parms.size() != 3) throw BaseException("Missing parameter(s) - should be /clan-join clan player");

		std::string clanName = wide_to_narrow(ctx->parms[1]);
		u16 clan = ctx->env->clansManager.clanId(clanName);
		if(!clan) throw BaseException("Bad clan name or clan deleted");

		std::string playerName = wide_to_narrow(ctx->parms[2]);
		if(playerName.length()==0) throw BaseException("Bad player name");

		if(!ctx->player->isClanModerator(clan)) throw BaseException("Only clan moderator can do this");

		Player* player = ctx->env->getPlayer(playerName.c_str());

		if(!player) throw BaseException("Bad player name or player disconnected");

		player->clans.insert(clan);
		ctx->server->SendPlayerClan(player,false,clan);

		os<< L"-!- clan-join - success";
		ctx->server->BroadcastChatMessage(L"-!- Player " + ctx->parms[2] + L" joined clan " + ctx->parms[1]);

	}catch(BaseException& ex){
		std::wstring msg = narrow_to_wide(std::string(ex.what()));
		os << L"-!- Error - player not added to clan (" << msg << L")";
	}
	
}

void cmd_clanKick(std::wostringstream &os,
	ServerCommandContext *ctx)
{
	try{
		if((ctx->privs & PRIV_CLANS) == 0) throw BaseException("You don't have permission to do that");

		if(ctx->parms.size() != 3) throw BaseException("Missing parameter(s) - should be /clan-kick clan player");
	
		std::string clanName = wide_to_narrow(ctx->parms[1]);
		u16 clan = ctx->env->clansManager.clanId(clanName);
		if(!clan) throw BaseException("Bad clan name or clan deleted"); //clan must exist

		if(!ctx->player->isClanModerator(clan)) throw BaseException("Only clan moderator can do this"); //sender must be moderator of this clan

		std::string playerName = wide_to_narrow(ctx->parms[2]);
		if(playerName.length()==0) throw BaseException("Bad player name");
		Player* player = ctx->env->getPlayer(playerName.c_str());
		if(!player) throw BaseException("Bad player name or player disconnected"); //player must exist

		if(player->clanOwner == clan) throw BaseException("Clan's owner cannot be kicked out"); //player can't be owner of that clan

		player->clans.erase(clan);
		player->clansModerator.erase(clan);
		ctx->server->SendPlayerClan(player,true,clan);

		os<< L"-!- clan-kick - success";
		ctx->server->BroadcastChatMessage(L"-!- Player " + ctx->parms[2] + L" kicked from clan " + ctx->parms[1]);

	}catch(BaseException& ex){
		std::wstring msg = narrow_to_wide(std::string(ex.what()));
		os << L"-!- Error - player not kicked from clan. (" << msg << L")";
	}
	
}

void cmd_clanPromote(std::wostringstream &os,
	ServerCommandContext *ctx)
{
	try{
		if((ctx->privs & PRIV_CLANS) == 0) throw BaseException("You don't have permission to do that");

		if(ctx->parms.size() != 3) throw BaseException("Missing parameter(s) - should be /clan-promote clan player");

		std::string clanName = wide_to_narrow(ctx->parms[1]);
		u16 clan = ctx->env->clansManager.clanId(clanName);
		if(!clan) throw BaseException("Bad clan name or clan deleted"); //clan must exist

		if(!ctx->player->isClanOwner(clan)) throw BaseException("Only clan owner can do this"); //must be admin

		std::string playerName = wide_to_narrow(ctx->parms[2]);
		if(playerName.length()==0) throw BaseException("Bad player name");

		Player* player = ctx->env->getPlayer(playerName.c_str());
		if(!player) throw BaseException("Bad player name or player disconnected"); //player must exist

		if(!player->isClanMember(clan)){
			player->clans.insert(clan);
			ctx->server->SendPlayerClan(player,false,clan);
		}

		player->clansModerator.insert(clan);

		os<< L"-!- clan-promote - success";
		ctx->server->BroadcastChatMessage(L"-!- Player " + ctx->parms[2] + L" was promoted in clan " + ctx->parms[1]);

	}catch(BaseException& ex){
		std::wstring msg = narrow_to_wide(std::string(ex.what()));
		os << L"-!- Error - can't promote player. (" << msg << L")";
	}
	
}

void cmd_clanDegrade(std::wostringstream &os,
	ServerCommandContext *ctx)
{
	try{
		if((ctx->privs & PRIV_CLANS) == 0) throw BaseException("You don't have permission to do that");

		if(ctx->parms.size() != 3) throw BaseException("Missing parameter(s) - should be /clan-degrade clan player");

		std::string clanName = wide_to_narrow(ctx->parms[1]);
		u16 clan = ctx->env->clansManager.clanId(clanName);
		if(!clan) throw BaseException("Bad clan name or clan deleted"); //clan must exist

		if(!ctx->player->isClanOwner(clan)) throw BaseException("Only clan owner can do this"); //must be admin

		std::string playerName = wide_to_narrow(ctx->parms[2]);
		if(playerName.length()==0) throw BaseException("Bad player name");

		Player* player = ctx->env->getPlayer(playerName.c_str());
		if(!player) throw BaseException("Bad player name or player disconnected"); //player must exist

		if(!player->isClanModerator(clan)) throw BaseException("Player is not a moderator");

		player->clansModerator.erase(clan);

		os<< L"-!- clan-degrade - success";
		ctx->server->BroadcastChatMessage(L"-!- Player " + ctx->parms[2] + L" was degraded in clan " + ctx->parms[1]);

	}catch(BaseException& ex){
		std::wstring msg = narrow_to_wide(std::string(ex.what()));
		os << L"-!- Error - can't degrade player. (" << msg << L")";
	}
	
}

void cmd_clanSpawn(std::wostringstream &os,
	ServerCommandContext *ctx)
{
	try{
		if((ctx->privs & PRIV_CLANS) == 0) throw BaseException("You don't have permission to do that");

		if(ctx->parms.size() != 2) throw BaseException("Missing parameter(s) - should be /clan-spawn clan");

		std::string clanName = wide_to_narrow(ctx->parms[1]);
		u16 clan = ctx->env->clansManager.clanId(clanName);
		if(!clan) throw BaseException("Bad clan name or clan deleted"); //clan must exist

		if(!ctx->player->isClanModerator(clan)) throw BaseException("Only clan moderator can do this"); //must be admin

		v3f pos = ctx->player->getPosition();
		ctx->env->clansManager.getClan(clan)->setSpawnPoint(pos);

		//ctx->server->BroadcastClanSpawn(clan,pos);

		os << L"-!- clan-spawn - success";

	}catch(BaseException& ex){
		std::wstring msg = narrow_to_wide(std::string(ex.what()));
		os << L"-!- Error - can't change clan's spawn point. (" << msg << L")";
	}	
}

void cmd_clanInfo(std::wostringstream &os,
	ServerCommandContext *ctx)
{
	try{
		if((ctx->privs & PRIV_CLANS) == 0) throw BaseException("You don't have permission to do that");
		
		const ClansManager& cm = ctx->env->clansManager;
		const Player& p = *ctx->player;

		if(p.clans.size()>0){
			os << L"-!- Your clans: ";

			for(std::set<u16>::const_iterator it=p.clans.begin(); it!=p.clans.end(); it++){
				if(!cm.clanExists(*it))continue;
				if(it!=p.clans.begin()) os << L", ";
				os << narrow_to_wide(cm.clanNameNoEx(*it));
				if(p.isClanOwner(*it)) os << L" (owner)";
				else if(p.isClanModerator(*it)) os << L" (moderator)";
			}
		}else os << L"You don't belong to any clan.";

	}catch(BaseException& ex){
		std::wstring msg = narrow_to_wide(std::string(ex.what()));
		os << L"-!- Error - can't get clans info. (" << msg << L")";
	}
	
}

std::wstring processServerCommand(ServerCommandContext *ctx)
{

	std::wostringstream os(std::ios_base::binary);
	ctx->flags = SEND_TO_SENDER;	// Default, unless we change it.

	u64 privs = ctx->privs;

	if(ctx->parms.size() == 0 || ctx->parms[0] == L"help")
	{
		os<<L"-!- Available commands: ";
		os<<L"status privs ";
		if(privs & PRIV_SERVER)
			os<<L"shutdown setting ";
		if(privs & PRIV_SETTIME)
			os<<L" time";
		if(privs & PRIV_TELEPORT)
			os<<L" teleport";
		if(privs & PRIV_PRIVS)
			os<<L" grant revoke";
		if(privs & PRIV_BAN)
			os<<L" ban unban";
	}
	else if(ctx->parms[0] == L"status")
		cmd_status(os, ctx);
	else if(ctx->parms[0] == L"privs")
		cmd_privs(os, ctx);
	else if(ctx->parms[0] == L"grant" || ctx->parms[0] == L"revoke")
		cmd_grantrevoke(os, ctx);
	else if(ctx->parms[0] == L"time")
		cmd_time(os, ctx);
	else if(ctx->parms[0] == L"shutdown")
		cmd_shutdown(os, ctx);
	else if(ctx->parms[0] == L"setting")
		cmd_setting(os, ctx);
	else if(ctx->parms[0] == L"teleport")
		cmd_teleport(os, ctx);
	else if(ctx->parms[0] == L"ban" || ctx->parms[0] == L"unban")
		cmd_banunban(os, ctx);
	else if(ctx->parms[0] == L"me")
		cmd_me(os, ctx);
	else if(ctx->parms[0] == L"clearobjects")
		cmd_clearobjects(os, ctx);
	else if(ctx->parms[0] == L"clan-new")
		cmd_clanNew(os, ctx);
	else if(ctx->parms[0] == L"clan-delete")
		cmd_clanDelete(os, ctx);
	else if(ctx->parms[0] == L"clan-join")
		cmd_clanJoin(os, ctx);
	else if(ctx->parms[0] == L"clan-kick")
		cmd_clanKick(os, ctx);
	else if(ctx->parms[0] == L"clan-promote")
		cmd_clanPromote(os, ctx);
	else if(ctx->parms[0] == L"clan-degrade")
		cmd_clanDegrade(os, ctx);
	else if(ctx->parms[0] == L"clan-spawn")
		cmd_clanSpawn(os, ctx);
	else if(ctx->parms[0] == L"clan-info")
		cmd_clanInfo(os, ctx);
	else
		os<<L"-!- Invalid command: " + ctx->parms[0];
	
	return os.str();
}


