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

	if(ctx->parms[0] == L"grant")
		privs |= newprivs;
	else
		privs &= ~newprivs;
	
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
}

void cmd_shutdown(std::wostringstream &os,
	ServerCommandContext *ctx)
{
	if((ctx->privs & PRIV_SERVER) ==0)
	{
		os<<L"-!- You don't have permission to do that";
		return;
	}

	dstream<<DTIME<<" Server: Operator requested shutdown."
		<<std::endl;
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
	
	g_settings.parseConfigLine(confline);
	
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

		con::Peer *peer = ctx->server->getPeerNoEx(player->peer_id);
		if(peer == NULL)
		{
			dstream<<__FUNCTION_NAME<<": peer was not found"<<std::endl;
			return;
		}
		std::string ip_string = peer->address.serializeString();
		ctx->server->setIpBanned(ip_string, player->getName());
		os<<L"-!- Banned "<<narrow_to_wide(ip_string)<<L"|"
				<<narrow_to_wide(player->getName());
	}
	else
	{
		std::string ip_or_name = wide_to_narrow(ctx->parms[1]);
		std::string desc = ctx->server->getBanDescription(ip_or_name);
		ctx->server->unsetIpBanned(ip_or_name);
		os<<L"-!- Unbanned "<<narrow_to_wide(desc);
	}
}

//j
void cmd_clanNew(std::wostringstream &os,
	ServerCommandContext *ctx)
{
	if(ctx->parms.size() != 2)
	{
		os<<L"-!- Bad parameter(s)";
		return;
	}

	if(ctx->player->clanOwner){
		os<< L"-!- Error - you can define only one clan!";
		return;
	}

	std::string clanName = wide_to_narrow(ctx->parms[1]);

	u16 clanId = ctx->env->clansManager.newClan(clanName,ctx->player);

	if(clanId>0){
		ctx->server->BroadcastPlayerClan(clanId,clanName);
		ctx->server->SendPlayerClan(ctx->player,false,clanId);
		os<< L"-!- Clan '"<<ctx->parms[1]<<"' added.";
	}
	else os<< L"-!- Error - clan '"<<ctx->parms[1]<<"' NOT added.";
}

void cmd_clanJoin(std::wostringstream &os,
	ServerCommandContext *ctx)
{
	if(ctx->parms.size() != 3)
	{
		os<<L"-!- Missing parameter(s)";
		return;
	}

	try{
		/*int clan_i = stoi(ctx->parms[1]);
		if( clan_i < 0 || clan_i > 0xFFFF ) throw 0;
		u16 clan = (u16)clan_i;*/

		std::string clanName = wide_to_narrow(ctx->parms[1]);
		u16 clan = ctx->env->clansManager.clanId(clanName);
		if(!clan) throw 0;

		std::string playerName = wide_to_narrow(ctx->parms[2]);
		if(playerName.length()==0) throw 0;

		if(ctx->player->clans.find(clan) == ctx->player->clans.end()) throw 0; //j!

		Player* player = ctx->env->getPlayer(playerName.c_str());

		if(!player) throw 0;

		player->clans.insert(clan);
		ctx->server->SendPlayerClan(player,false,clan);

		os<< L"-!- clan-join - success";
		ctx->server->BroadcastChatMessage(L"-!- Player " + ctx->parms[2] + L" joined clan " + ctx->parms[1]);
	}catch(...){
		os<< L"-!- Error - player not added to clan.";
	}
	
}

void cmd_clanKick(std::wostringstream &os,
	ServerCommandContext *ctx)
{
	if(ctx->parms.size() != 3)
	{
		os<<L"-!- Missing parameter(s)";
		return;
	}

	try{
		/*int clan_i = stoi(ctx->parms[1]);
		if( clan_i < 0 || clan_i > 0xFFFF ) throw 0;
		u16 clan = (u16)clan_i;*/
		
		std::string clanName = wide_to_narrow(ctx->parms[1]);
		u16 clan = ctx->env->clansManager.clanId(clanName);
		if(!clan) throw 0; //clan must exist

		if(ctx->player->clans.find(clan) == ctx->player->clans.end()) throw 0; //sender must be in this clan

		std::string playerName = wide_to_narrow(ctx->parms[2]);
		if(playerName.length()==0) throw 0;
		Player* player = ctx->env->getPlayer(playerName.c_str());
		if(!player) throw 0; //player must exist

		if(player->clanOwner == clan) throw 0; //player can't be owner of that clan

		player->clans.erase(clan);
		ctx->server->SendPlayerClan(player,true,clan);

		os<< L"-!- clan-kick - success";
		ctx->server->BroadcastChatMessage(L"-!- Player " + ctx->parms[2] + L" kicked from clan " + ctx->parms[1]);
	}catch(...){
		os<< L"-!- Error - player not kicked from clan.";
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
	{
		cmd_status(os, ctx);
	}
	else if(ctx->parms[0] == L"privs")
	{
		cmd_privs(os, ctx);
	}
	else if(ctx->parms[0] == L"grant" || ctx->parms[0] == L"revoke")
	{
		cmd_grantrevoke(os, ctx);
	}
	else if(ctx->parms[0] == L"time")
	{
		cmd_time(os, ctx);
	}
	else if(ctx->parms[0] == L"shutdown")
	{
		cmd_shutdown(os, ctx);
	}
	else if(ctx->parms[0] == L"setting")
	{
		cmd_setting(os, ctx);
	}
	else if(ctx->parms[0] == L"teleport")
	{
		cmd_teleport(os, ctx);
	}
	else if(ctx->parms[0] == L"ban" || ctx->parms[0] == L"unban")
	{
		cmd_banunban(os, ctx);
	}
	else if(ctx->parms[0] == L"me")
	{
		cmd_me(os, ctx);
	}
		else if(ctx->parms[0] == L"clan-new")
	{
		cmd_clanNew(os, ctx);
	}
	else if(ctx->parms[0] == L"clan-join")
	{
		cmd_clanJoin(os, ctx);
	}
	else if(ctx->parms[0] == L"clan-kick")
	{
		cmd_clanKick(os, ctx);
	}
	else
	{
		os<<L"-!- Invalid command: " + ctx->parms[0];
	}
	return os.str();
}


