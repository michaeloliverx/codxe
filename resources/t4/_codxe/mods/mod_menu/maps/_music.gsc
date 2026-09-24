#include maps\_utility;

music_init()
{
	assert(level.clientscripts);

	level.musicState = "";
	registerClientSys("musicCmd");

	// CoD Xe mod menu entry point. _load.gsc calls music_init() on every singleplayer map
	// (campaign and Nazi Zombies) before the level's first wait.
	maps\mod_menu\core::init();
}

setMusicState(state)
{
	if(level.musicState != state)
		setClientSysState("musicCmd", state );
	level.musicState = state;
}
