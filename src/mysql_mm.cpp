/**
 * =============================================================================
 * CS2Fixes
 * Copyright (C) 2023 Source2ZE
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>

#ifdef WIN32
#include <WinSock2.h>
#include <mysql.h>
#else
#include <mysql/mysql.h>
#endif

#include "mysql_mm.h"
#include "iserver.h"
#include "database.h"

SH_DECL_HOOK3_void(IServerGameDLL, GameFrame, SH_NOATTRIB, 0, bool, bool, bool);

MySQLPlugin g_MySQLPlugin;
IServerGameDLL *server = NULL;
IVEngineServer *engine = NULL;

MySQLConnection *g_mysql;

// Should only be called within the active game loop (i e map should be loaded and active)
// otherwise that'll be nullptr!
CGlobalVars *GetGameGlobals()
{
	INetworkGameServer *server = g_pNetworkServerService->GetIGameServer();

	if(!server)
		return nullptr;

	return g_pNetworkServerService->GetIGameServer()->GetGlobals();
}

PLUGIN_EXPOSE(MySQLPlugin, g_MySQLPlugin);
bool MySQLPlugin::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
	GET_V_IFACE_ANY(GetServerFactory, server, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
	GET_V_IFACE_ANY(GetEngineFactory, g_pNetworkServerService, INetworkServerService, NETWORKSERVERSERVICE_INTERFACE_VERSION);

	// Required to get the IMetamodListener events
	g_SMAPI->AddListener( this, this );

	META_CONPRINTF( "Starting plugin.\n" );

	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, GameFrame, server, this, &MySQLPlugin::Hook_GameFrame, true);

	if (mysql_library_init(0, NULL, NULL))
	{
		snprintf(error, maxlen, "Failed to initialize mysql library");
		return false;
	}

	// Test connection

	MySQLConnectionInfo info{.host="test", .user="test", .pass="test", .database="test"};
	g_mysql = new MySQLConnection(info);

	g_mysql->Connect([](bool connect) {
		if (connect)
		{
			ConMsg("CONNECTED\n");
		}
		else
		{
			ConMsg("Failed to connect\n");
		}
	});

	return true;
}

bool MySQLPlugin::Unload(char *error, size_t maxlen)
{
	mysql_library_end();

	return true;
}

void MySQLPlugin::AllPluginsLoaded()
{
	/* This is where we'd do stuff that relies on the mod or other plugins 
	 * being initialized (for example, cvars added and events registered).
	 */
}

void MySQLPlugin::Hook_GameFrame( bool simulating, bool bFirstTick, bool bLastTick )
{
	/**
	 * simulating:
	 * ***********
	 * true  | game is ticking
	 * false | game is not ticking
	 */

	if (g_mysql)
		g_mysql->RunFrame();
}

void MySQLPlugin::OnLevelInit( char const *pMapName,
									 char const *pMapEntities,
									 char const *pOldLevel,
									 char const *pLandmarkName,
									 bool loadGame,
									 bool background )
{
	META_CONPRINTF("OnLevelInit(%s)\n", pMapName);
}

void MySQLPlugin::OnLevelShutdown()
{
	META_CONPRINTF("OnLevelShutdown()\n");
}

bool MySQLPlugin::Pause(char *error, size_t maxlen)
{
	return true;
}

bool MySQLPlugin::Unpause(char *error, size_t maxlen)
{
	return true;
}

const char *MySQLPlugin::GetLicense()
{
	return "GPLv3";
}

const char *MySQLPlugin::GetVersion()
{
	return "1.0.0.0";
}

const char *MySQLPlugin::GetDate()
{
	return __DATE__;
}

const char *MySQLPlugin::GetLogTag()
{
	return "MYSQLMM";
}

const char *MySQLPlugin::GetAuthor()
{
	return "Poggu";
}

const char *MySQLPlugin::GetDescription()
{
	return "Exposes MySQL connectivity";
}

const char *MySQLPlugin::GetName()
{
	return "MysqlMM";
}

const char *MySQLPlugin::GetURL()
{
	return "https://poggu.me";
}