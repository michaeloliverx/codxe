
#include maps\_utility;

music_init()
{
	assert(level.clientscripts);

	level.musicState = "";
	registerClientSys("musicCmd");

	// Entry point for our custom module
	thread maps\custom::init();
}

setMusicState(state)
{
	if(level.musicState != state)
		setClientSysState("musicCmd", state );
	level.musicState = state;
}
