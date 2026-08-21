#include maps\mp\_utility;
#include maps\mp\gametypes\_hud_util;

main()
{
	maps\mp\gametypes\_globallogic::init();
	maps\mp\gametypes\_callbacksetup::SetupCallbacks();
	maps\mp\gametypes\_globallogic::SetupCallbacks();

	maps\mp\gametypes\_globallogic::registerTimeLimitDvar( level.gameType, 10, 0, 1440 );
	maps\mp\gametypes\_globallogic::registerScoreLimitDvar( level.gameType, 1000, 0, 5000 );
	maps\mp\gametypes\_globallogic::registerRoundLimitDvar( level.gameType, 1, 0, 10 );
	maps\mp\gametypes\_globallogic::registerNumLivesDvar( level.gameType, 0, 0, 10 );

	level.onStartGameType = ::onStartGameType;
	level.onSpawnPlayer = ::onSpawnPlayer;

	game["dialog"]["gametype"] = "freeforall";
}


onStartGameType()
{
	setClientNameMode("auto_change");

	maps\mp\gametypes\_globallogic::setObjectiveText( "allies", &"OBJECTIVES_DM" );
	maps\mp\gametypes\_globallogic::setObjectiveText( "axis", &"OBJECTIVES_DM" );

	if ( level.splitscreen )
	{
		maps\mp\gametypes\_globallogic::setObjectiveScoreText( "allies", &"OBJECTIVES_DM" );
		maps\mp\gametypes\_globallogic::setObjectiveScoreText( "axis", &"OBJECTIVES_DM" );
	}
	else
	{
		maps\mp\gametypes\_globallogic::setObjectiveScoreText( "allies", &"OBJECTIVES_DM_SCORE" );
		maps\mp\gametypes\_globallogic::setObjectiveScoreText( "axis", &"OBJECTIVES_DM_SCORE" );
	}
	maps\mp\gametypes\_globallogic::setObjectiveHintText( "allies", &"OBJECTIVES_DM_HINT" );
	maps\mp\gametypes\_globallogic::setObjectiveHintText( "axis", &"OBJECTIVES_DM_HINT" );

	level.spawnMins = ( 0, 0, 0 );
	level.spawnMaxs = ( 0, 0, 0 );
	maps\mp\gametypes\_spawnlogic::addSpawnPoints( "allies", "mp_dm_spawn" );
	maps\mp\gametypes\_spawnlogic::addSpawnPoints( "axis", "mp_dm_spawn" );
	level.mapCenter = maps\mp\gametypes\_spawnlogic::findBoxCenter( level.spawnMins, level.spawnMaxs );
	setMapCenter( level.mapCenter );
	
	allowed[0] = "dm";
	maps\mp\gametypes\_gameobjects::main(allowed);

	maps\mp\gametypes\_rank::registerScoreInfo( "kill", 5 );
	maps\mp\gametypes\_rank::registerScoreInfo( "headshot", 5 );
	maps\mp\gametypes\_rank::registerScoreInfo( "assist", 1 );
	maps\mp\gametypes\_rank::registerScoreInfo( "suicide", 0 );
	maps\mp\gametypes\_rank::registerScoreInfo( "teamkill", 0 );
	
	level.displayRoundEndText = false;
	level.QuickMessageToAll = true;

	// elimination style
	if ( level.roundLimit != 1 && level.numLives )
	{
		level.overridePlayerScore = true;
		level.displayRoundEndText = true;
		level.onEndGame = ::onEndGame;
	}

	thread maps\mp\gametypes\cj::init();
}


onSpawnPlayer()
{
	spawnPoints = maps\mp\gametypes\_spawnlogic::getTeamSpawnPoints( self.pers["team"] );
	spawnPoint = maps\mp\gametypes\_spawnlogic::getSpawnpoint_DM( spawnPoints );

	self spawn( spawnPoint.origin, spawnPoint.angles );
}


onEndGame( winningPlayer )
{
	if ( isDefined( winningPlayer ) )
		[[level._setPlayerScore]]( winningPlayer, winningPlayer [[level._getPlayerScore]]() + 1 );	
}
