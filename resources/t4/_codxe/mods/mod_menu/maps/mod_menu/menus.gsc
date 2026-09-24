/*
	Menu tree. Built once, the first time anyone opens the menu, so level scripts (zombies
	setup, loadouts, spawners) have finished initialising. Menus with a builder are rebuilt from
	live level data every time they are entered.
*/

#include maps\_utility;
#include common_scripts\utility;
#include maps\mod_menu\util;
#include maps\mod_menu\core;

build()
{
	zombies = mm_is_zombies();

	mm_menu("main", "Main Menu", undefined);
	mm_add_sub("main", "Player", "player");
	mm_add_sub("main", "Weapons", "weapons");
	mm_add_sub("main", "Bullets & Aim", "bullets");
	mm_add_sub("main", "Fun & Chaos", "fun");
	mm_add_sub("main", "Enemies", "ai");
	if (zombies)
		mm_add_sub("main", "Nazi Zombies", "zombies");
	mm_add_sub("main", "World & Physics", "world");
	mm_add_sub("main", "Visuals", "visuals");
	mm_add_sub("main", "Forge", "forge");
	if (!zombies)
		mm_add_sub("main", "Death Cards", "cards");
	mm_add_sub("main", "Players", "players");
	mm_add_sub("main", "Menu Settings", "settings");

	build_player();
	build_weapons();
	build_bullets();
	build_fun();
	build_ai(zombies);
	build_world();
	build_visuals();
	build_forge();
	build_cards();
	build_zombies();
	build_players();
	build_settings();

	level.mm.built = true;
}

// ---------------------------------------------------------------------------

build_player()
{
	mm_menu("player", "Player", "main");
	mm_add_toggle("player", "God Mode", "god", maps\mod_menu\player::god_set, "self", maps\mod_menu\player::god_get, true);
	mm_add_toggle("player", "Demigod", "demigod", maps\mod_menu\player::demigod_set, "self", undefined, true);
	mm_add_toggle("player", "Noclip", "noclip", maps\mod_menu\player::noclip_set, "self", maps\mod_menu\player::noclip_get, true);
	mm_add_toggle("player", "UFO Mode", "ufo", maps\mod_menu\player::ufo_set, "self", maps\mod_menu\player::ufo_get, true);
	mm_add_toggle("player", "Invisible", "invisible", maps\mod_menu\player::invisible_set, "self", undefined, true);
	mm_add_toggle("player", "Infinite Ammo", "ammo", maps\mod_menu\player::infinite_ammo_set);
	speed = mm_add_slider("player", "Move Speed", "speed", maps\mod_menu\player::speed_set, 0.5, 4, 0.25, 1);
	level.mm.reapply[level.mm.reapply.size] = speed;
	mm_add_toggle("player", "Super Jump", "superjump", maps\mod_menu\player::super_jump_set, "level");
	mm_add_toggle("player", "No Fall Damage", "no_fall", maps\mod_menu\player::no_fall_damage_set, "level");
	mm_add_toggle("player", "Unlimited Sprint", "sprint", maps\mod_menu\player::sprint_set, "level");
	mm_add_toggle("player", "Third Person", "thirdperson", maps\mod_menu\player::third_person_set, "self", undefined, true);
	mm_add_slider("player", "Field of View", "fov", maps\mod_menu\player::fov_set, 65, 120, 5, 65);
	mm_add_action("player", "Max Health 1000", maps\mod_menu\player::max_health);
	mm_add_sub("player", "Teleport", "teleport");

	mm_menu("teleport", "Teleport", "player");
	mm_add_action("teleport", "To Crosshair", maps\mod_menu\player::teleport_crosshair);
	mm_add_action("teleport", "Save Position", maps\mod_menu\player::save_position);
	mm_add_action("teleport", "Load Position", maps\mod_menu\player::load_position);
	mm_add_action("teleport", "Behind An Enemy", maps\mod_menu\player::teleport_to_enemy);
	mm_add_action("teleport", "Skydive", maps\mod_menu\player::skydive);
}

build_weapons()
{
	mm_menu("weapons", "Weapons", "main");
	mm_add_sub("weapons", "Give Weapon", "give_weapon");
	mm_add_action("weapons", "Random Weapon", maps\mod_menu\weapons::give_random_weapon);
	mm_add_action("weapons", "Refill Ammo", maps\mod_menu\weapons::refill_ammo);
	mm_add_action("weapons", "Take Current Weapon", maps\mod_menu\weapons::take_current_weapon);
	if (mm_is_zombies())
		mm_add_action("weapons", "Pack-a-Punch", maps\mod_menu\weapons::pack_a_punch);
	mm_add_toggle("weapons", "Rapid Fire", "rapidfire", maps\mod_menu\weapons::rapid_fire_set);
	mm_add_toggle("weapons", "Fast Reload", "fastreload", maps\mod_menu\weapons::fast_reload_set);
	mm_add_toggle("weapons", "Cluster Grenades", "sf_cluster", maps\mod_menu\world::cheat_cluster_set, "level");
	mm_add_toggle("weapons", "Engine Infinite Ammo", "sf_ammo", maps\mod_menu\world::cheat_ammo_set, "level");

	mm_menu("give_weapon", "Give Weapon", "weapons");
	mm_set_builder("give_weapon", ::fill_give_weapon);
}

fill_give_weapon(key)
{
	mm_clear(key);
	pool = self maps\mod_menu\weapons::weapon_pool();
	for (i = 0; i < pool.size; i++)
		mm_add_action(key, mm_short(pool[i], 24), maps\mod_menu\weapons::give_weapon, pool[i]);
}

build_bullets()
{
	mm_menu("bullets", "Bullets & Aim", "main");
	mm_add_choice("bullets", "Bullet Mode", "bullets", maps\mod_menu\weapons::bullet_mode_set, maps\mod_menu\weapons::bullet_mode_names());
	mm_add_toggle("bullets", "Aimbot (hold LT)", "aimbot", maps\mod_menu\weapons::aimbot_set);
	mm_add_toggle("bullets", "Death Stare", "stare", maps\mod_menu\weapons::death_stare_set);
}

build_fun()
{
	mm_menu("fun", "Fun & Chaos", "main");
	mm_add_toggle("fun", "Chaos Mode", "chaos", maps\mod_menu\chaos::chaos_set, "level");
	mm_add_slider("fun", "Chaos Interval", "chaos_interval", maps\mod_menu\chaos::chaos_interval_set, 5, 60, 5, 20, "level");
	mm_add_toggle("fun", "Jetpack (hold A)", "jetpack", maps\mod_menu\fun::jetpack_set);
	mm_add_action("fun", "Rocket Ride", maps\mod_menu\fun::rocket_ride);
	mm_add_action("fun", "Human Cannonball", maps\mod_menu\fun::human_cannonball);
	mm_add_toggle("fun", "Ground Pound", "pound", maps\mod_menu\fun::ground_pound_set);
	mm_add_toggle("fun", "Force Field", "field", maps\mod_menu\fun::force_field_set);
	mm_add_toggle("fun", "Matrix Mode", "matrix", maps\mod_menu\fun::matrix_set, "level");
	mm_add_toggle("fun", "Drunk Mode", "drunk", maps\mod_menu\fun::drunk_set);
	mm_add_toggle("fun", "Disco Mode", "disco", maps\mod_menu\fun::disco_set, "level");
	mm_add_action("fun", "Tactical Nuke", maps\mod_menu\fun::nuke);
	mm_add_action("fun", "Meteor Shower", maps\mod_menu\fun::meteor_shower);
	mm_add_action("fun", "The Rapture", maps\mod_menu\fun::rapture);
	mm_add_action("fun", "Blood Rain", maps\mod_menu\fun::blood_rain);
	mm_add_action("fun", "Earthquake", maps\mod_menu\fun::earthquake_now);
	mm_add_sub("fun", "FX Browser", "fx");

	mm_menu("fx", "FX Browser", "fun");
	mm_set_builder("fx", ::fill_fx);
}

fill_fx(key)
{
	mm_clear(key);
	keys = maps\mod_menu\fun::fx_keys();
	for (i = 0; i < keys.size; i++)
		mm_add_action(key, mm_short(keys[i], 24), maps\mod_menu\fun::fx_play, keys[i]);
}

build_ai(zombies)
{
	mm_menu("ai", "Enemies", "main");
	mm_add_action("ai", "Kill All", maps\mod_menu\ai::kill_all);
	mm_add_action("ai", "Gib All", maps\mod_menu\ai::gib_all);
	mm_add_action("ai", "Launch All", maps\mod_menu\ai::launch_all);
	mm_add_action("ai", "Bring To Crosshair", maps\mod_menu\ai::teleport_enemies_here);
	if (!zombies)
	{
		mm_add_action("ai", "Switch Sides", maps\mod_menu\ai::switch_sides);
		mm_add_action("ai", "Civil War", maps\mod_menu\ai::civil_war);
	}
	mm_add_toggle("ai", "Freeze", "freeze", maps\mod_menu\ai::freeze_set, "level");
	if (!zombies)
		mm_add_toggle("ai", "Pacifist", "pacifist", maps\mod_menu\ai::pacifist_set, "level");
	mm_add_toggle("ai", "One Hit Kills", "onehit", maps\mod_menu\ai::one_hit_set, "level");
	mm_add_toggle("ai", "Stormtrooper Aim", "stormtrooper", maps\mod_menu\ai::stormtrooper_set, "level");
	mm_add_slider("ai", "Enemy Speed", "ai_speed", maps\mod_menu\ai::speed_set, 0.25, 3, 0.25, 1, "level");
	if (!zombies)
	{
		mm_add_toggle("ai", "Invincible Squad", "friendly_god", maps\mod_menu\ai::friendly_god_set, "level");
		mm_add_action("ai", "Spawn Bodyguard", maps\mod_menu\ai::bodyguard);
	}
	mm_add_sub("ai", "Spawn Enemy", "spawn_ai");

	mm_menu("spawn_ai", "Spawn Enemy", "ai");
	mm_set_builder("spawn_ai", ::fill_spawn_ai);
}

fill_spawn_ai(key)
{
	mm_clear(key);
	types = maps\mod_menu\ai::spawner_types();
	for (i = 0; i < types.size; i++)
		mm_add_action(key, maps\mod_menu\ai::spawner_label(types[i]), maps\mod_menu\ai::spawn_type_at_crosshair, types[i]);
}

build_world()
{
	mm_menu("world", "World & Physics", "main");
	// Script time runs on game time, so the menu itself slows down with the timescale; 0.2 keeps it usable.
	mm_add_slider("world", "Timescale", "timescale", maps\mod_menu\world::timescale_set, 0.2, 3, 0.1, 1, "level");
	mm_add_slider("world", "Gravity", "gravity", maps\mod_menu\world::gravity_set, 50, 1600, 50, 800, "level");
	mm_add_choice("world", "Ragdoll Gravity", "physgrav", maps\mod_menu\world::phys_gravity_set, maps\mod_menu\world::phys_gravity_names(), "level");
	mm_add_toggle("world", "Hurricane Winds", "wind", maps\mod_menu\world::wind_set, "level");
	mm_add_action("world", "Detonate Destructibles", maps\mod_menu\world::destroy_destructibles);
	mm_add_action("world", "Destroy Vehicles", maps\mod_menu\world::destroy_vehicles);
	mm_add_action("world", "Drivable Vehicles", maps\mod_menu\world::vehicles_usable);
}

build_visuals()
{
	mm_menu("visuals", "Visuals", "main");
	mm_add_sub("visuals", "Vision Sets", "visions");
	mm_add_choice("visuals", "Fog", "fog", maps\mod_menu\world::fog_set, maps\mod_menu\world::fog_names(), "level");
	mm_add_choice("visuals", "Sun", "sun", maps\mod_menu\world::sun_set, maps\mod_menu\world::sun_names(), "level");
	mm_add_toggle("visuals", "Black & White", "sf_bw", maps\mod_menu\world::cheat_bw_set, "level");
	mm_add_toggle("visuals", "Photo Negative", "sf_invert", maps\mod_menu\world::cheat_invert_set, "level");
	mm_add_toggle("visuals", "Super Contrast", "sf_contrast", maps\mod_menu\world::cheat_contrast_set, "level");
	mm_add_toggle("visuals", "Silent Film", "sf_chaplin", maps\mod_menu\world::cheat_chaplin_set, "level");
	mm_add_toggle("visuals", "Slow-mo Ability", "sf_slowmo", maps\mod_menu\world::cheat_slowmo_set, "level");
	mm_add_toggle("visuals", "Fullbright", "fullbright", maps\mod_menu\world::fullbright_set);
	mm_add_toggle("visuals", "Motion Blur", "motionblur", maps\mod_menu\world::motion_blur_set, "level");
	mm_add_toggle("visuals", "Double Vision", "doublevision", maps\mod_menu\world::double_vision_set);
	mm_add_toggle("visuals", "Film Grain", "grain", maps\mod_menu\fun::film_grain_set);
	mm_add_toggle("visuals", "Hide HUD", "hidehud", maps\mod_menu\world::hide_hud_set, "level");
	mm_add_toggle("visuals", "Hide Gun", "hidegun", maps\mod_menu\world::hide_gun_set);

	mm_menu("visions", "Vision Sets", "visuals");
	mm_set_builder("visions", ::fill_visions);
}

fill_visions(key)
{
	mm_clear(key);
	names = maps\mod_menu\world::vision_names();
	for (i = 0; i < names.size; i++)
		mm_add_action(key, mm_short(names[i], 24), maps\mod_menu\world::vision_set, names[i]);
}

build_forge()
{
	mm_menu("forge", "Forge", "main");
	mm_add_sub("forge", "Spawn Model", "models");
	mm_add_toggle("forge", "Physics Spawns", "forge_physics", maps\mod_menu\forge::physics_set);
	mm_add_toggle("forge", "Grab Mode (hold LB)", "pickup", maps\mod_menu\forge::pickup_set);
	mm_add_action("forge", "Launch Aimed", maps\mod_menu\forge::launch_aimed);
	mm_add_action("forge", "Delete Aimed", maps\mod_menu\forge::delete_aimed);
	mm_add_action("forge", "Undo Last Spawn", maps\mod_menu\forge::undo_spawn);
	mm_add_action("forge", "Clear My Spawns", maps\mod_menu\forge::clear_spawns);

	mm_menu("models", "Spawn Model", "forge");
	mm_set_builder("models", ::fill_models);
}

fill_models(key)
{
	mm_clear(key);
	models = maps\mod_menu\forge::model_list();
	for (i = 0; i < models.size; i++)
		mm_add_action(key, mm_short(models[i], 24), maps\mod_menu\forge::spawn_model, models[i]);
}

build_cards()
{
	mm_menu("cards", "Death Cards", "main");
	mm_add_toggle("cards", "Thunder", "card_thunder", maps\mod_menu\world::card_thunder_set, "level", maps\mod_menu\world::card_thunder_get);
	mm_add_toggle("cards", "Paintball", "card_paintball", maps\mod_menu\world::card_paintball_set, "level", maps\mod_menu\world::card_paintball_get);
	mm_add_toggle("cards", "Cold Dead Hands", "card_dead_hands", maps\mod_menu\world::card_dead_hands_set, "level", maps\mod_menu\world::card_dead_hands_get);
	mm_add_toggle("cards", "Undead", "card_zombie", maps\mod_menu\world::card_zombie_set, "level", maps\mod_menu\world::card_zombie_get);
	mm_add_toggle("cards", "Hard Headed", "card_hard_headed", maps\mod_menu\world::card_hard_headed_set, "level", maps\mod_menu\world::card_hard_headed_get);
	mm_add_toggle("cards", "Berserker", "card_berserker", maps\mod_menu\world::card_berserker_set, "level", maps\mod_menu\world::card_berserker_get);
	mm_add_toggle("cards", "Vampire", "card_vampire", maps\mod_menu\world::card_vampire_set, "level", maps\mod_menu\world::card_vampire_get);
	mm_add_toggle("cards", "Sticks and Stones", "card_sticks", maps\mod_menu\world::card_sticks_set, "level", maps\mod_menu\world::card_sticks_get);
	mm_add_toggle("cards", "Flak Jacket", "card_flak", maps\mod_menu\world::card_flak_set, "level", maps\mod_menu\world::card_flak_get);
	mm_add_toggle("cards", "Body Armor", "card_armor", maps\mod_menu\world::card_armor_set, "level", maps\mod_menu\world::card_armor_get);
	mm_add_toggle("cards", "Morphine Shot", "card_morphine", maps\mod_menu\world::card_morphine_set, "level", maps\mod_menu\world::card_morphine_get);
	mm_add_toggle("cards", "Dirty Harry", "card_harry", maps\mod_menu\world::card_harry_set, "level", maps\mod_menu\world::card_harry_get);
	mm_add_toggle("cards", "Hardcore", "card_hardcore", maps\mod_menu\world::card_hardcore_set, "level", maps\mod_menu\world::card_hardcore_get);
}

build_zombies()
{
	mm_menu("zombies", "Nazi Zombies", "main");
	mm_add_sub("zombies", "Points", "zm_points");
	mm_add_action("zombies", "Give All Perks", maps\mod_menu\zombies::give_all_perks);
	mm_add_sub("zombies", "Perks", "zm_perks");
	mm_add_action("zombies", "Open All Doors", maps\mod_menu\zombies::open_all_doors);
	mm_add_action("zombies", "Turn On Power", maps\mod_menu\zombies::power_on);
	mm_add_sub("zombies", "Drop Power-Up Now", "zm_drop");
	mm_add_sub("zombies", "Rig Next Drop", "zm_rig");
	mm_add_action("zombies", "Skip Round", maps\mod_menu\zombies::skip_round);
	jump = mm_add_slider("zombies", "Jump To Round (A)", "zm_round", maps\mod_menu\zombies::jump_to_round, 1, 100, 1, 10, "self", false);
	jump.no_reset = true;
	mm_add_choice("zombies", "Zombie Speed", "zm_speed", maps\mod_menu\zombies::speed_set, maps\mod_menu\zombies::speed_names(), "level");
	mm_add_action("zombies", "Crawler Army", maps\mod_menu\zombies::crawlers);
	mm_add_action("zombies", "Headless Horde", maps\mod_menu\zombies::headless);
	mm_add_action("zombies", "Revive Everyone", maps\mod_menu\zombies::revive_all);
	mm_add_toggle("zombies", "Zombie Counter", "zm_counter", maps\mod_menu\zombies::zombie_counter_set);

	mm_menu("zm_points", "Points", "zombies");
	mm_add_action("zm_points", "+1,000", maps\mod_menu\zombies::add_points, 1000);
	mm_add_action("zm_points", "+10,000", maps\mod_menu\zombies::add_points, 10000);
	mm_add_action("zm_points", "+100,000", maps\mod_menu\zombies::add_points, 100000);
	mm_add_action("zm_points", "Reset To 500", maps\mod_menu\zombies::reset_points);
	mm_add_toggle("zm_points", "Infinite Points", "zm_infinite", maps\mod_menu\zombies::infinite_points_set);

	mm_menu("zm_perks", "Perks", "zombies");
	mm_add_action("zm_perks", "Juggernog", maps\mod_menu\zombies::give_perk, "specialty_armorvest");
	mm_add_action("zm_perks", "Quick Revive", maps\mod_menu\zombies::give_perk, "specialty_quickrevive");
	mm_add_action("zm_perks", "Speed Cola", maps\mod_menu\zombies::give_perk, "specialty_fastreload");
	mm_add_action("zm_perks", "Double Tap", maps\mod_menu\zombies::give_perk, "specialty_rof");

	mm_menu("zm_drop", "Drop Power-Up Now", "zombies");
	mm_set_builder("zm_drop", ::fill_zm_drop);
	mm_menu("zm_rig", "Rig Next Drop", "zombies");
	mm_set_builder("zm_rig", ::fill_zm_rig);
}

fill_zm_drop(key)
{
	mm_clear(key);
	names = maps\mod_menu\zombies::powerup_names();
	for (i = 0; i < names.size; i++)
		mm_add_action(key, names[i], maps\mod_menu\zombies::drop_now, names[i]);
}

fill_zm_rig(key)
{
	mm_clear(key);
	names = maps\mod_menu\zombies::powerup_names();
	for (i = 0; i < names.size; i++)
		mm_add_action(key, names[i], maps\mod_menu\zombies::next_drop, names[i]);
}

build_players()
{
	mm_menu("players", "Players", "main");
	mm_set_builder("players", ::fill_players);

	mm_menu("player_opts", "Player Options", "players");
	mm_add_action("player_opts", "Bring To Me", maps\mod_menu\players::bring_here);
	mm_add_action("player_opts", "Go To Them", maps\mod_menu\players::go_to);
	mm_add_action("player_opts", "Toggle God Mode", maps\mod_menu\players::give_god);
	mm_add_action("player_opts", "Toggle Menu Access", maps\mod_menu\players::give_menu);
	mm_add_action("player_opts", "Human Cannonball", maps\mod_menu\players::launch);
	mm_add_action("player_opts", "Revive", maps\mod_menu\players::revive);
	if (mm_is_zombies())
		mm_add_action("player_opts", "Give 10,000 Points", maps\mod_menu\players::give_points);
}

fill_players(key)
{
	mm_clear(key);
	players = get_players();
	for (i = 0; i < players.size; i++)
		mm_add_action(key, self maps\mod_menu\players::player_label(players[i]), maps\mod_menu\players::select_player, players[i]);
}

build_settings()
{
	mm_menu("settings", "Menu Settings", "main");
	theme = mm_add_choice("settings", "Theme", "theme", ::mm_set_theme, level.mm.theme_names);
	theme.no_reset = true;
	sides = [];
	sides[0] = "Right";
	sides[1] = "Left";
	side = mm_add_choice("settings", "Menu Side", "side", ::mm_set_side, sides);
	side.no_reset = true;
	combo = mm_add_choice("settings", "Open With", "combo", ::mm_set_combo, mm_combo_names());
	combo.no_reset = true;
	mm_add_action("settings", "Controls Help", ::mm_controls_help);
	mm_add_toggle("settings", "All Players Get Menu", "all_access", ::mm_set_all_access, "level");
	mm_add_action("settings", "Reset All Mods", ::mm_reset_all);
	mm_add_action("settings", "About", ::mm_about);
}
