/*
	CoD Xe mod menu - engine

	Entry point is maps\_music::music_init(), which _load.gsc calls on every singleplayer map
	(campaign and Nazi Zombies) before the level script's first wait, so precaching here is safe.

	HUD budget notes
	- Every distinct string passed to setText() uses a localized-string config slot until the map
	  ends. The list is drawn as one text element per page (not per row) and values use setValue()
	  or a small fixed set of strings, so the number of distinct strings stays bounded.
	- A HUD string is limited to ~255 characters, so labels stay short and pages hold 10 rows.
*/

#include maps\_utility;
#include common_scripts\utility;
#include maps\mod_menu\util;

init()
{
	precacheShader("white");

	level.mm = spawnStruct();
	level.mm.title = "CoD Xe Menu";
	level.mm.menus = [];
	level.mm.built = false;
	level.mm.rows = 10;
	level.mm.row_h = 15; // "default" font line height is 12 * fontScale
	level.mm.font_scale = 1.25;
	level.mm.width = 250;
	level.mm.all_access = false;
	level.mm.dpad = false;
	level.mm.reapply = [];
	level.mm_state = [];

	level.mm.theme_names = [];
	level.mm.theme_colors = [];
	mm_add_theme("Cyan", (0, 0.75, 1));
	mm_add_theme("Crimson", (0.85, 0.08, 0.15));
	mm_add_theme("Toxic", (0.35, 1, 0.1));
	mm_add_theme("Gold", (1, 0.75, 0.1));
	mm_add_theme("Violet", (0.55, 0.2, 1));
	mm_add_theme("Orange", (1, 0.45, 0));
	mm_add_theme("Pink", (1, 0.25, 0.6));
	mm_add_theme("Ice", (0.75, 0.9, 1));

	level thread mm_watch_connect("connecting");
	level thread mm_watch_connect("connected");
}

mm_add_theme(name, color)
{
	level.mm.theme_names[level.mm.theme_names.size] = name;
	level.mm.theme_colors[level.mm.theme_colors.size] = color;
}

// ---------------------------------------------------------------------------
// Player lifecycle
// ---------------------------------------------------------------------------

mm_watch_connect(msg)
{
	for (;;)
	{
		level waittill(msg, player);
		player thread mm_player_init();
	}
}

mm_player_init()
{
	if (isDefined(self.mm_init))
		return;
	self.mm_init = true;

	self endon("disconnect");

	self.mm_state = [];
	self.mm_cursor = [];
	self.mm_hold = [];
	self.mm_hud = [];
	self.mm_open = false;
	self.mm_menu = "main";
	self.mm_theme = 0;
	self.mm_side = 0;
	self.mm_access = false;
	self.mm_host = false;
	self.mm_wait_release = false;
	self.mm_lowered = false;

	while (!isAlive(self))
		wait 0.1;

	self mm_load_settings();

	if (self mm_is_host())
	{
		self.mm_host = true;
		self.mm_access = true;
		self thread mm_probe_dpad();
	}

	self thread mm_spawn_watch();
	self.mm_heartbeat = getTime();
	self thread mm_input_loop();
	self thread mm_input_watchdog();

	wait 3;
	if (self mm_has_access())
	{
		self iprintln("^5CoD Xe Menu ^7loaded");
		self iprintln(self mm_open_hint());
	}
}

// ---------------------------------------------------------------------------
// Per-player settings. Stored in dvars so they survive loading the next level.
// ---------------------------------------------------------------------------

mm_setting_dvar(name)
{
	return "mm_" + name + "_" + self getEntityNumber();
}

mm_load_settings()
{
	self.mm_theme = mm_clamp(getDvarInt(self mm_setting_dvar("theme")), 0, level.mm.theme_names.size - 1);
	self.mm_side = mm_clamp(getDvarInt(self mm_setting_dvar("side")), 0, 1);
	self.mm_combo = mm_clamp(getDvarInt(self mm_setting_dvar("combo")), 0, 1);
	self.mm_state["theme"] = self.mm_theme;
	self.mm_state["side"] = self.mm_side;
	self.mm_state["combo"] = self.mm_combo;
}

mm_save_setting(name, value)
{
	setDvar(self mm_setting_dvar(name), value);
}

mm_combo_names()
{
	names = [];
	names[0] = "LT + RS";
	names[1] = "Crouch + RS";
	return names;
}

mm_open_hint()
{
	if (self.mm_combo == 1)
		return "Crouch (^3B^7) and press ^3RS^7 (melee) to open the menu";
	return "Hold ^3LT^7 (aim) and press ^3RS^7 (melee) to open the menu";
}

// Uses the aim/melee/stance *actions*, so it follows whatever button layout the player picked.
mm_open_combo_pressed()
{
	if (!self meleeButtonPressed())
		return false;
	if (self.mm_combo == 1)
		return self getStance() == "crouch";
	return self adsButtonPressed();
}

mm_has_access()
{
	return self.mm_access || level.mm.all_access;
}

// ButtonPressed() reads the local pad; on some builds it may be developer only. If it throws,
// only this thread dies and D-pad navigation simply stays disabled.
mm_probe_dpad()
{
	self endon("disconnect");
	wait 1;
	self buttonPressed("DPAD_UP");
	level.mm.dpad = true;
}

mm_dpad(key)
{
	if (!level.mm.dpad || !self.mm_host)
		return false;
	return self buttonPressed(key);
}

mm_spawn_watch()
{
	self endon("disconnect");
	for (;;)
	{
		self waittill("spawned_player");
		self thread mm_reapply();
	}
}

// Engine fields (god, noclip, ...) and move speed can be reset by a respawn; restore them.
mm_reapply()
{
	self endon("disconnect");
	wait 0.5;
	for (i = 0; i < level.mm.reapply.size; i++)
	{
		item = level.mm.reapply[i];
		if (isDefined(self.mm_state[item.key]) && self.mm_state[item.key])
			self thread [[item.func]](self.mm_state[item.key]);
	}
}

// ---------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------

// Handlers are started with `thread` so a script error inside one (for example a dynamic menu
// builder hitting an unexpected entity) only kills that handler, never this loop. None of them
// wait, so they still run to completion before the loop continues.
mm_input_loop()
{
	self endon("disconnect");
	self endon("mm_input_restart");

	for (;;)
	{
		wait 0.05;
		self.mm_heartbeat = getTime();

		if (!self mm_has_access())
		{
			if (self.mm_open)
				self mm_close();
			continue;
		}

		if (!self.mm_open)
		{
			// Closing with RS must not instantly reopen while LT is still held.
			if (!self meleeButtonPressed())
				self.mm_wait_release = false;
			if (!self.mm_wait_release && isAlive(self) && self mm_open_combo_pressed())
			{
				self.mm_wait_release = true;
				self thread mm_open_menu();
			}
			continue;
		}

		if (!isAlive(self))
		{
			self mm_close();
			continue;
		}

		if (self mm_button("back", self meleeButtonPressed(), false))
		{
			self thread mm_back();
			continue;
		}

		// Holding RS closes the menu from any page.
		if (self.mm_hold["back"] == 12)
		{
			self thread mm_close();
			continue;
		}

		if (self mm_button("select", self jumpButtonPressed() || self useButtonPressed(), false))
		{
			self thread mm_select();
			continue;
		}

		up = self adsButtonPressed() || self moveForwardButtonPressed() || self mm_dpad("DPAD_UP");
		down = self attackButtonPressed() || self moveBackButtonPressed() || self mm_dpad("DPAD_DOWN");
		left = self secondaryOffhandButtonPressed() || self moveLeftButtonPressed() || self mm_dpad("DPAD_LEFT");
		right = self fragButtonPressed() || self moveRightButtonPressed() || self mm_dpad("DPAD_RIGHT");

		if (self mm_button("up", up, true))
			self thread mm_move(-1);
		if (self mm_button("down", down, true))
			self thread mm_move(1);
		if (self mm_button("left", left, true))
			self thread mm_adjust(-1);
		if (self mm_button("right", right, true))
			self thread mm_adjust(1);
	}
}

// Restarts the input loop if it ever stops beating (e.g. killed by an unexpected script error).
mm_input_watchdog()
{
	self endon("disconnect");
	for (;;)
	{
		wait 1;
		if (getTime() - self.mm_heartbeat > 1500)
		{
			self notify("mm_input_restart");
			self.mm_heartbeat = getTime();
			self thread mm_input_loop();
		}
	}
}

// Edge detection with auto-repeat. A negative hold count means "ignore until released".
mm_button(name, pressed, repeat)
{
	if (!isDefined(self.mm_hold[name]))
		self.mm_hold[name] = 0;

	if (!pressed)
	{
		self.mm_hold[name] = 0;
		return false;
	}

	ticks = self.mm_hold[name];
	self.mm_hold[name] = ticks + 1;

	if (ticks == 0)
		return true;
	if (repeat && ticks >= 7 && (ticks % 2) == 1)
		return true;
	return false;
}

mm_block_held_buttons()
{
	names = [];
	names[names.size] = "back";
	names[names.size] = "select";
	names[names.size] = "up";
	names[names.size] = "down";
	names[names.size] = "left";
	names[names.size] = "right";
	for (i = 0; i < names.size; i++)
		self.mm_hold[names[i]] = -1000;
}

// ---------------------------------------------------------------------------
// Open / close
// ---------------------------------------------------------------------------

mm_open_menu()
{
	if (!level.mm.built)
		maps\mod_menu\menus::build();

	if (!isDefined(level.mm.menus[self.mm_menu]))
		self.mm_menu = "main";

	self.mm_open = true;
	self freezeControls(true);
	self disableOffhandWeapons();

	// Lower the gun so RT (scroll down) can never fire. Vehicles and turrets manage their own
	// weapon state, so leave those alone.
	if (!self.usingvehicle && !self.usingturret)
	{
		self disableWeapons();
		self.mm_lowered = true;
	}

	self mm_create_hud();
	self mm_enter(self.mm_menu);
	self mm_block_held_buttons();
}

mm_close()
{
	self.mm_open = false;
	self.mm_wait_release = true;
	self mm_destroy_hud();
	self freezeControls(false);
	self enableOffhandWeapons();
	if (self.mm_lowered)
	{
		self enableWeapons();
		self.mm_lowered = false;
	}
	self notify("mm_closed");
}

// ---------------------------------------------------------------------------
// HUD
// ---------------------------------------------------------------------------

mm_theme_color()
{
	return level.mm.theme_colors[self.mm_theme];
}

mm_hud_rect(x, y, width, height, color, alpha, sort)
{
	elem = newClientHudElem(self);
	elem.x = x;
	elem.y = y;
	elem.alignX = "left";
	elem.alignY = "top";
	elem.horzAlign = "fullscreen";
	elem.vertAlign = "fullscreen";
	elem.foreground = true;
	elem.hidewheninmenu = true;
	elem.archived = false;
	elem.sort = sort;
	elem.color = color;
	elem setShader("white", int(width), int(height));
	elem.alpha = 0;
	elem fadeOverTime(0.15);
	elem.alpha = alpha;
	return elem;
}

mm_hud_text(x, y, alignX, fontScale, sort)
{
	elem = newClientHudElem(self);
	elem.x = x;
	elem.y = y;
	elem.alignX = alignX;
	elem.alignY = "top";
	elem.horzAlign = "fullscreen";
	elem.vertAlign = "fullscreen";
	elem.font = "default";
	elem.fontScale = fontScale;
	elem.foreground = true;
	elem.hidewheninmenu = true;
	elem.archived = false;
	elem.sort = sort;
	elem.color = (1, 1, 1);
	elem.alpha = 1;
	return elem;
}

mm_create_hud()
{
	self mm_destroy_hud();

	rows = level.mm.rows;
	rowH = level.mm.row_h;
	width = level.mm.width;
	color = self mm_theme_color();

	x = 640 - width - 18;
	if (self.mm_side == 1)
		x = 18;
	y = 60;
	listTop = y + 36;
	height = 36 + (rows + 1) * rowH + 26;

	self.mm_list_y = listTop + rowH; // first line of the list text is the page title

	hud = [];
	hud["bg"] = self mm_hud_rect(x, y, width, height, (0.02, 0.02, 0.04), 0.82, 1);
	hud["bar"] = self mm_hud_rect(x, y, width, 30, color, 0.85, 2);
	hud["edge"] = self mm_hud_rect(x, y + height - 2, width, 2, color, 0.85, 2);
	hud["scroller"] = self mm_hud_rect(x, self.mm_list_y, width, rowH, color, 0.35, 2);

	hud["header"] = self mm_hud_text(x + 10, y + 4, "left", 1.6, 4);
	hud["header"].font = "objective";
	hud["header"].glowColor = color;
	hud["header"].glowAlpha = 0.6;
	hud["header"] setText(level.mm.title);

	hud["list"] = self mm_hud_text(x + 10, listTop, "left", level.mm.font_scale, 4);

	for (r = 0; r < rows; r++)
		hud["val" + r] = self mm_hud_text(x + width - 10, self.mm_list_y + r * rowH, "right", level.mm.font_scale, 4);

	hud["footer"] = self mm_hud_text(x + 10, y + height - 20, "left", 0.9, 4);
	hud["footer"].alpha = 0.65;
	hud["footer"] setText("LT/RT Scroll  A Select  RS Back  LB/RB Adjust");

	self.mm_hud = hud;
	self.mm_page_key = "";
}

mm_destroy_hud()
{
	if (!isDefined(self.mm_hud))
		return;
	keys = getArrayKeys(self.mm_hud);
	for (i = 0; i < keys.size; i++)
	{
		if (isDefined(self.mm_hud[keys[i]]))
			self.mm_hud[keys[i]] destroy();
	}
	self.mm_hud = [];
}

mm_apply_theme()
{
	if (!self.mm_open)
		return;
	color = self mm_theme_color();
	self.mm_hud["bar"].color = color;
	self.mm_hud["edge"].color = color;
	self.mm_hud["scroller"].color = color;
	self.mm_hud["header"].glowColor = color;
}

mm_render()
{
	if (!self.mm_open)
		return;

	menu = level.mm.menus[self.mm_menu];
	count = menu.items.size;
	rows = level.mm.rows;

	cursor = self mm_cursor_get();
	page = int(cursor / rows);
	pages = int((count + rows - 1) / rows);
	if (pages < 1)
		pages = 1;
	top = page * rows;

	pageKey = self.mm_menu + "|" + page + "|" + menu.version;
	if (pageKey != self.mm_page_key)
	{
		self.mm_page_key = pageKey;
		self.mm_hud["list"] setText(mm_page_text(menu, page, pages, top));
	}

	for (r = 0; r < rows; r++)
	{
		elem = self.mm_hud["val" + r];
		if (top + r >= count)
			elem.alpha = 0;
		else
			self mm_render_value(elem, menu.items[top + r]);
	}

	scroller = self.mm_hud["scroller"];
	if (count == 0)
	{
		scroller.alpha = 0;
		return;
	}
	scroller.alpha = 0.35;
	scroller moveOverTime(0.06);
	scroller.y = self.mm_list_y + (cursor - top) * level.mm.row_h;
}

// One HUD string holds the page title and every visible label, and HUD strings are limited to
// about 255 characters, so shorten labels until the page fits.
mm_page_text(menu, page, pages, top)
{
	count = menu.items.size;
	header = "^3" + menu.title;
	if (pages > 1)
		header = header + " ^7(" + (page + 1) + "/" + pages + ")";
	header = header + "^7";
	if (count == 0)
		return header + "\n(nothing here)";

	for (maxLength = 24; maxLength >= 8; maxLength -= 2)
	{
		text = header;
		for (r = 0; r < level.mm.rows && top + r < count; r++)
		{
			label = menu.items[top + r].label;
			if (label.size > maxLength)
				label = getSubStr(label, 0, maxLength);
			text = text + "\n" + label;
		}
		if (text.size <= 250)
			return text;
	}
	return text;
}

mm_render_value(elem, item)
{
	switch (item.type)
	{
	case "toggle":
		if (self mm_item_value(item))
		{
			elem setText("ON");
			elem.color = (0.35, 1, 0.45);
		}
		else
		{
			elem setText("OFF");
			elem.color = (1, 0.35, 0.35);
		}
		elem.alpha = 1;
		break;
	case "slider":
		elem setValue(self mm_item_value(item));
		elem.color = (1, 1, 1);
		elem.alpha = 1;
		break;
	case "choice":
		index = self mm_item_value(item);
		if (!isDefined(index) || index < 0 || index >= item.choices.size)
			index = 0;
		elem setText(item.choices[index]);
		elem.color = (1, 0.9, 0.5);
		elem.alpha = 1;
		break;
	case "submenu":
		elem setText(">");
		elem.color = (1, 1, 1);
		elem.alpha = 0.6;
		break;
	default:
		elem.alpha = 0;
		break;
	}
}

// ---------------------------------------------------------------------------
// Navigation
// ---------------------------------------------------------------------------

mm_cursor_get()
{
	menu = level.mm.menus[self.mm_menu];
	if (!isDefined(self.mm_cursor[self.mm_menu]))
		self.mm_cursor[self.mm_menu] = 0;
	if (self.mm_cursor[self.mm_menu] >= menu.items.size)
		self.mm_cursor[self.mm_menu] = 0;
	return self.mm_cursor[self.mm_menu];
}

mm_current_item()
{
	menu = level.mm.menus[self.mm_menu];
	if (menu.items.size == 0)
		return undefined;
	return menu.items[self mm_cursor_get()];
}

mm_enter(key)
{
	if (!isDefined(level.mm.menus[key]))
	{
		self iprintln("^1Menu '" + key + "' does not exist");
		return;
	}

	menu = level.mm.menus[key];
	if (isDefined(menu.builder))
		self [[menu.builder]](key);

	self.mm_menu = key;
	self.mm_page_key = "";
	self mm_render();
}

mm_back()
{
	menu = level.mm.menus[self.mm_menu];
	if (isDefined(menu.parent))
		self mm_enter(menu.parent);
	else
		self mm_close();
}

mm_move(delta)
{
	menu = level.mm.menus[self.mm_menu];
	count = menu.items.size;
	if (count == 0)
		return;
	self.mm_cursor[self.mm_menu] = (self mm_cursor_get() + delta + count) % count;
	self mm_render();
}

mm_select()
{
	item = self mm_current_item();
	if (!isDefined(item))
		return;

	switch (item.type)
	{
	case "submenu":
		self mm_enter(item.target);
		break;
	case "action":
		self thread mm_run_action(item);
		break;
	case "toggle":
		self mm_set_toggle(item, !self mm_item_value(item));
		break;
	case "slider":
		self thread [[item.func]](self mm_item_value(item));
		break;
	case "choice":
		self mm_adjust(1);
		break;
	default:
		break;
	}
}

mm_run_action(item)
{
	self endon("disconnect");
	if (isDefined(item.arg2))
		self [[item.func]](item.arg, item.arg2);
	else if (isDefined(item.arg))
		self [[item.func]](item.arg);
	else
		self [[item.func]]();
}

mm_adjust(dir)
{
	item = self mm_current_item();
	if (!isDefined(item))
		return;

	if (item.type == "slider")
	{
		value = mm_round(mm_clamp(self mm_item_value(item) + dir * item.step, item.min, item.max));
		self mm_item_set(item, value);
		if (item.live)
			self thread [[item.func]](value);
		self mm_render();
	}
	else if (item.type == "choice")
	{
		count = item.choices.size;
		index = (self mm_item_value(item) + dir + count) % count;
		self mm_item_set(item, index);
		self thread [[item.func]](index);
		self mm_render();
	}
	else if (item.type == "toggle")
	{
		self mm_set_toggle(item, dir > 0);
	}
}

mm_set_toggle(item, on)
{
	if (on)
		on = true;
	else
		on = false;
	self mm_item_set(item, on);
	self thread [[item.func]](on);
	self mm_render();
}

mm_round(value)
{
	if (value >= 0)
		return int(value * 100 + 0.5) / 100;
	return int(value * 100 - 0.5) / 100;
}

// ---------------------------------------------------------------------------
// Item state
// ---------------------------------------------------------------------------

mm_item_value(item)
{
	if (isDefined(item.getter))
		return self [[item.getter]]();

	if (item.scope == "level")
		value = level.mm_state[item.key];
	else
		value = self.mm_state[item.key];

	if (!isDefined(value))
		value = item.def;
	return value;
}

mm_item_set(item, value)
{
	if (item.scope == "level")
		level.mm_state[item.key] = value;
	else
		self.mm_state[item.key] = value;
}

// Turn every toggle off and put sliders/choices back to their defaults.
mm_reset_all()
{
	keys = getArrayKeys(level.mm.menus);
	for (m = 0; m < keys.size; m++)
	{
		items = level.mm.menus[keys[m]].items;
		for (i = 0; i < items.size; i++)
		{
			item = items[i];
			if (!isDefined(item.key) || isDefined(item.no_reset))
				continue;
			value = self mm_item_value(item);
			if (item.type == "toggle" && value)
			{
				self mm_item_set(item, false);
				self thread [[item.func]](false);
			}
			else if ((item.type == "slider" || item.type == "choice") && value != item.def)
			{
				self mm_item_set(item, item.def);
				self thread [[item.func]](item.def);
			}
		}
	}
	self.mm_page_key = "";
	self mm_render();
	self iprintln("^5All mods reset");
}

// ---------------------------------------------------------------------------
// Menu construction API (used by menus.gsc)
// ---------------------------------------------------------------------------

mm_menu(key, title, parent)
{
	menu = spawnStruct();
	menu.key = key;
	menu.title = title;
	menu.parent = parent;
	menu.items = [];
	menu.version = 0;
	level.mm.menus[key] = menu;
	return menu;
}

mm_clear(key)
{
	menu = level.mm.menus[key];
	menu.items = [];
	menu.version++;
}

mm_set_builder(key, func)
{
	level.mm.menus[key].builder = func;
}

mm_item(key, label, type)
{
	item = spawnStruct();
	item.label = label;
	item.type = type;
	item.scope = "self";
	item.def = 0;
	menu = level.mm.menus[key];
	menu.items[menu.items.size] = item;
	return item;
}

mm_add_sub(key, label, target)
{
	item = mm_item(key, label, "submenu");
	item.target = target;
	return item;
}

mm_add_action(key, label, func, arg, arg2)
{
	item = mm_item(key, label, "action");
	item.func = func;
	item.arg = arg;
	item.arg2 = arg2;
	return item;
}

mm_add_toggle(key, label, stateKey, func, scope, getter, reapply)
{
	item = mm_item(key, label, "toggle");
	item.key = stateKey;
	item.func = func;
	item.def = false;
	if (isDefined(scope))
		item.scope = scope;
	item.getter = getter;
	if (isDefined(reapply) && reapply)
		level.mm.reapply[level.mm.reapply.size] = item;
	return item;
}

mm_add_slider(key, label, stateKey, func, low, high, step, def, scope, live)
{
	item = mm_item(key, label, "slider");
	item.key = stateKey;
	item.func = func;
	item.min = low;
	item.max = high;
	item.step = step;
	item.def = def;
	if (isDefined(scope))
		item.scope = scope;
	item.live = !isDefined(live) || live;
	return item;
}

mm_add_choice(key, label, stateKey, func, choices, scope)
{
	item = mm_item(key, label, "choice");
	item.key = stateKey;
	item.func = func;
	item.choices = choices;
	item.def = 0;
	if (isDefined(scope))
		item.scope = scope;
	return item;
}

// ---------------------------------------------------------------------------
// Settings callbacks
// ---------------------------------------------------------------------------

mm_set_theme(index)
{
	self.mm_theme = index;
	self mm_save_setting("theme", index);
	self mm_apply_theme();
}

mm_set_side(index)
{
	self.mm_side = index;
	self mm_save_setting("side", index);
	if (!self.mm_open)
		return;
	self mm_create_hud();
	self mm_render();
}

mm_set_combo(index)
{
	self.mm_combo = index;
	self mm_save_setting("combo", index);
	self iprintln(self mm_open_hint());
}

mm_controls_help()
{
	self iprintln("^3Scroll^7: LT / RT, left stick or D-pad");
	self iprintln("^3Select^7: A (X also works)   ^3Back^7: RS   ^3Close^7: hold RS");
	self iprintln("^3Change values^7: LB / RB or left stick left / right");
	self iprintln(self mm_open_hint());
}

mm_set_all_access(on)
{
	level.mm.all_access = on;
	self mm_onoff("Menu for all players", on);
}

mm_about()
{
	self iprintlnbold("^5CoD Xe Menu");
	self iprintln("GSC injected by CoD Xe on Call of Duty: World at War (Xbox 360)");
	self iprintln("Campaign + Nazi Zombies. Every feature is map-agnostic.");
}
