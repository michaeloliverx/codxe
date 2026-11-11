// Generate an isolated full mod from the user's exact checkout. Never write into the source mod or game directory.
const fs = require('fs');
const path = require('path');
const crypto = require('crypto');
const root = path.resolve(__dirname, '../../..');
const source = path.resolve(process.argv[2] || 'C:/Users/Michael/code/codxenon/iw3_bot_warfare_xenon');
const output = path.join(root, 'build/iw3-native-bots/mod');
const adapter = fs.readFileSync(path.join(source, 'scripts/mp/bots_adapter_cod4x.gsc'), 'utf8');
if (!/do_botmovement\( forward, right \)\s*\{\s*\}/.test(adapter))
    throw new Error('This experiment requires the console adapter with an empty do_botmovement');
if (!output.startsWith(root + path.sep) || source === output || source.startsWith(output + path.sep))
    throw new Error('Generated output must be inside this workspace and separate from the input');
fs.rmSync(output, {recursive: true, force: true});
fs.mkdirSync(output, {recursive: true});
for (const dir of ['maps', 'scripts', 'scriptdata'])
    fs.cpSync(path.join(source, dir), path.join(output, dir), {recursive: true});
const hashes = {};
function read(relative) {
    const data = fs.readFileSync(path.join(source, relative));
    hashes[relative] = crypto.createHash('sha256').update(data).digest('hex');
    return data.toString('utf8').replace(/\r\n/g, '\n');
}
function replace(text, from, to) {
    if (!text.includes(from) || text.indexOf(from) !== text.lastIndexOf(from))
        throw new Error('Expected one source match: ' + from.slice(0, 100));
    return text.replace(from, to);
}
function write(relative, text) { fs.writeFileSync(path.join(output, relative), text.replace(/\n/g, '\r\n')); }
const utilityPath = 'maps/mp/bots/_bot_utility.gsc';
let utility = read(utilityPath);
utility = replace(utility, 'load_waypoints()\n{', 'load_waypoints()\n{\n\tBotNativeInit();');
utility = replace(utility, '\t\tlevel.waypoints[ i ].childcount = undefined;\n\t}\n}',
    '\t\tlevel.waypoints[ i ].childcount = undefined;\n\t}\n\tBotNativeUpload();\n}');
utility = replace(utility, 'getNearestWaypoint( pos )\n{', `getNearestWaypoint( pos )
{
	if ( BotNativePathEnabled() )
		return botnavnearest( pos );`);
// The nearest visible result and ties are unchanged; reject farther candidates before doing a trace.
utility = replace(utility, 'GetNearestWaypointWithSight( pos )\n{',
    'GetNearestWaypointWithSight( pos )\n{\n\tnative_path = BotNativePathEnabled();');
utility = replace(utility, `		if ( !bullettracepassed( pos + ( 0, 0, 15 ), level.waypoints[ i ].origin + ( 0, 0, 15 ), false, undefined ) )`,
    `		if ( native_path && distancesquared( level.waypoints[ i ].origin, pos ) > dist )
			continue;
		if ( !bullettracepassed( pos + ( 0, 0, 15 ), level.waypoints[ i ].origin + ( 0, 0, 15 ), false, undefined ) )`);
utility = replace(utility, 'AStarSearch( start, goal, team, greedy_path )\n{', 'AStarSearch_script( start, goal, team, greedy_path )\n{');
utility = replace(utility, '\t\t\t\t\tlevel.waypointusage[ team ][ bestNodeStr ]++;',
    '\t\t\t\t\tlevel.waypointusage[ team ][ bestNodeStr ]++;\n\t\t\t\t\tBotNativeUsage( bestNode.index, team, 1 );');
utility = replace(utility, '\tlevel.waypointusage[ team ][ wpstr ]--;',
    '\tlevel.waypointusage[ team ][ wpstr ]--;\n\tBotNativeUsage( wp, team, -1 );');
utility = replace(utility, '\tself.bot.isfrozen = what;', '\tself.bot.isfrozen = what;\n\tif ( what )\n\t\tself botnativeaimstop();');
utility += `
// Native controller is latched at map load. Set it to zero before loading to compare the old hybrid.
BotNativeInit()
{
	level.native_nav_ready = false;
	if ( getdvar( "bots_native_controller" ) == "" )
		setdvar( "bots_native_controller", 1 );
	level.native_controller = getdvarint( "bots_native_controller" ) && !getdvarint( "bots_main_debug" );
	if ( getdvar( "bots_native_drive" ) == "" )
		setdvar( "bots_native_drive", 1 );
	if ( getdvar( "bots_native_sight_budget" ) == "" )
		setdvar( "bots_native_sight_budget", 6 );
	if ( getdvar( "bots_native_path_budget" ) == "" )
		setdvar( "bots_native_path_budget", 1 );
	if ( getdvar( "bots_native_reaction_ms" ) == "" )
		setdvar( "bots_native_reaction_ms", 250 );
	if ( getdvar( "bots_native_range" ) == "" )
		setdvar( "bots_native_range", 4000 );
	if ( getdvar( "bots_native_fov" ) == "" )
		setdvar( "bots_native_fov", 180 );
	if ( getdvar( "bots_native_turn_rate" ) == "" )
		setdvar( "bots_native_turn_rate", 300 );
	if ( level.native_controller )
		level thread BotNativeMatchWatch();
	if ( getdvar( "bots_native_path" ) == "" )
		setdvar( "bots_native_path", 1 );
	if ( getdvar( "bots_native_aim" ) == "" )
		setdvar( "bots_native_aim", 1 );
	if ( getdvar( "bots_native_profile" ) == "" )
		setdvar( "bots_native_profile", 1 );
	if ( getdvar( "bots_native_sight_prefilter" ) == "" )
		setdvar( "bots_native_sight_prefilter", 1 );
	if ( getdvar( "bots_native_sight_order" ) == "" )
		setdvar( "bots_native_sight_order", 1 );
	level.native_profile = getdvarint( "bots_native_profile" );
	if ( !isdefined( level.native_profile_started ) && level.native_profile )
	{
		level.native_profile_started = true;
		botnativeprofilereset();
		level thread BotNativeProfileReport();
	}
}

BotNativePathEnabled()
{
	return isdefined( level.native_nav_ready ) && level.native_nav_ready && getdvarint( "bots_native_path" );
}

BotNativeUpload()
{
	level.native_nav_ready = false;
	botnavbegin( 0 );
	// Editor graphs mutate while playing. Use the script implementation throughout editor sessions.
	if ( ( !getdvarint( "bots_native_path" ) && !level.native_controller ) || getdvarint( "bots_main_debug" ) || !botnavbegin( level.waypoints.size ) )
		return;
	for ( i = 0; i < level.waypoints.size; i++ )
	{
		wp = level.waypoints[ i ];
		extra = 0;
		if ( wp.type == "climb" || wp.type == "prone" )
			extra = 4;
		traversal = 0;
		if ( wp.type == "jump" )
			traversal = 1;
		if ( wp.type == "crouch" )
			traversal = 2;
		if ( wp.type == "prone" )
			traversal = 4;
		if ( wp.type == "climb" )
			traversal = 8;
		if ( !botnavnode( i, wp.origin, extra, traversal ) )
			return;
		for ( j = 0; j < wp.children.size; j++ )
			if ( !botnavlink( i, wp.children[ j ] ) )
				return;
	}
	level.native_nav_ready = botnavend();
}

BotNativeMatchWatch()
{
	botnativepause( true );
	for ( ;; )
	{
		if ( isdefined( level.gameended ) && level.gameended )
		{
			botnativepause( true );
			return;
		}
		paused = true;
		if ( isdefined( level.inprematchperiod ) )
			paused = level.inprematchperiod;
		botnativepause( paused );
		wait 0.25;
	}
}

BotNativeTeam( team )
{
	if ( isdefined( team ) && team == "allies" )
		return 0;
	if ( isdefined( team ) && team == "axis" )
		return 1;
	return -1;
}

BotNativeUsage( wp, team, delta )
{
	t = BotNativeTeam( team );
	if ( isdefined( level.native_nav_ready ) && level.native_nav_ready && t >= 0 )
		botnavusage( wp, t, delta );
}

BotNativeEndpoint( pos )
{
	wp = getNearestWaypoint( pos );
	if ( !isdefined( wp ) )
		return undefined;
	if ( !bullettracepassed( pos + ( 0, 0, 15 ), level.waypoints[ wp ].origin + ( 0, 0, 15 ), false, undefined ) )
	{
		visible = GetNearestWaypointWithSight( pos );
		if ( isdefined( visible ) )
			wp = visible;
	}
	return wp;
}

AStarSearch_native( start, goal, team, greedy_path )
{
	from = BotNativeEndpoint( start );
	to = BotNativeEndpoint( goal );
	if ( !isdefined( from ) || !isdefined( to ) )
		return [];
	if ( !isdefined( greedy_path ) )
		greedy_path = false;
	path = botnavsearch( from, to, BotNativeTeam( team ), greedy_path );
	if ( !isdefined( path ) )
		return undefined;
	if ( isdefined( team ) && isdefined( level.waypointusage ) )
	{
		for ( i = 0; i < path.size; i++ )
		{
			key = path[ i ] + "";
			if ( !isdefined( level.waypointusage[ team ][ key ] ) )
				level.waypointusage[ team ][ key ] = 0;
			level.waypointusage[ team ][ key ]++;
			BotNativeUsage( path[ i ], team, 1 );
		}
	}
	return path;
}

AStarSearch( start, goal, team, greedy_path )
{
	if ( isdefined( level.native_profile ) && level.native_profile )
		botnativeprofilebegin( 3 );
	result = undefined;
	if ( BotNativePathEnabled() )
		result = AStarSearch_native( start, goal, team, greedy_path );
	if ( !isdefined( result ) )
		result = AStarSearch_script( start, goal, team, greedy_path );
	if ( isdefined( level.native_profile ) && level.native_profile )
		botnativeprofileend( 3 );
	return result;
}

BotNativeProfileReport()
{
	level endon( "game_ended" );
	for ( ;; )
	{
		wait 10;
		// The native builtin writes directly to DbgPrint in Release as well as Debug.
		botnativeprofilestats();
		botnativeprofilereset();
	}
}
`;
write(utilityPath, utility);

const internalPath = 'maps/mp/bots/_bot_internal.gsc';
let internal = read(internalPath);
// Sight is already required by the original predicate. Reject a blocked sight ray before building a full trace.
internal = replace(internal,
    '\ttrace = bullettrace( myEye, boneLoc, false, undefined );\n\t\n\treturn ( sighttracepassed( myEye, boneLoc, false, undefined ) && ( trace[ "fraction" ] >= 1.0 || trace[ "surfacetype" ] == "glass" ) );',
    '\tif ( getdvarint( "bots_native_sight_order" ) )\n\t{\n\t\tif ( !sighttracepassed( myEye, boneLoc, false, undefined ) )\n\t\t\treturn false;\n\t\ttrace = bullettrace( myEye, boneLoc, false, undefined );\n\t\treturn ( trace[ "fraction" ] >= 1.0 || trace[ "surfacetype" ] == "glass" );\n\t}\n\ttrace = bullettrace( myEye, boneLoc, false, undefined );\n\t\n\treturn ( sighttracepassed( myEye, boneLoc, false, undefined ) && ( trace[ "fraction" ] >= 1.0 || trace[ "surfacetype" ] == "glass" ) );');
// Avoid movement trigonometry and normalization whose result is discarded by this Xbox adapter.
internal = replace(internal, '\tif ( distancesquared( self.origin, move_To ) >= 49 )',
    '\tif ( !getdvarint( "bots_native_path" ) && distancesquared( self.origin, move_To ) >= 49 )');
internal = replace(internal, '\tself BotBuiltinBotMovement( int( dir[ 0 ] ), int( dir[ 1 ] ) );',
    '\tif ( !getdvarint( "bots_native_path" ) )\n\t\tself BotBuiltinBotMovement( int( dir[ 0 ] ), int( dir[ 1 ] ) );');
// Reject candidates outside the same FOV or smoke test before fetching and tracing three bones.
// Both predicates are pure; the final visibility truth value and the forced-target override are unchanged.
const visibilityStart = internal.indexOf('\t\t\tcanTargetPlayer = ( ( player checkTraceForBone');
const visibilityEnd = internal.indexOf('\n\t\t\tif ( isdefined( self.bot.target_this_frame )', visibilityStart);
if (visibilityStart < 0 || visibilityEnd < 0) throw new Error('Missing visibility expression');
const originalVisibility = internal.slice(visibilityStart, visibilityEnd);
internal = internal.slice(0, visibilityStart) + '\t\t\tif ( getdvarint( "bots_native_sight_prefilter" ) )\n\t\t\t{\n\t\t\t\tcanTargetPlayer = ( ( getConeDot( player.origin, self.origin, myAngles ) >= myFov || ( isObjDef && obj.trace_time ) )\n\t\t\t\t\t&& ( SmokeTrace( myEye, player.origin, level.smokeradius ) || daDist < level.bots_maxknifedistance * 4 )\n\t\t\t\t\t&& ( player checkTraceForBone( myEye, "j_head" ) || player checkTraceForBone( myEye, "j_ankle_le" ) || player checkTraceForBone( myEye, "j_ankle_ri" ) ) );\n\t\t\t}\n\t\t\telse\n\t\t\t{\n' + originalVisibility + '\n\t\t\t}\n' + internal.slice(visibilityEnd);
// Always cancel a previous native request before accepting or rejecting its replacement.
internal = replace(internal, '\tself notify( "bots_aim_overlap" );',
    '\tself notify( "bots_aim_overlap" );\n\tself botnativeaimstop();');
internal = replace(internal, '\tsteps = int( time * 20 );', `	if ( getdvarint( "bots_native_aim" ) && time >= 0 && time <= 30 )
	{
		self botnativeaim( self getEyePos(), pos, time, vel, doAimPredict );
		return;
	}
	steps = int( time * 20 );`);
internal = replace(internal, '\t\tif ( level.inprematchperiod || level.gameended || self.bot.isfrozen || self maps\\mp\\_flashgrenades::isflashbanged() )', '\t\tif ( !getdvarint( "bots_native_aim" ) || level.gameended || level.inprematchperiod || self.bot.isfrozen || !getdvarint( "bots_play_aim" ) )\n\t\t\tself botnativeaimstop();\n\t\tif ( level.inprematchperiod || level.gameended || self.bot.isfrozen || self maps\\mp\\_flashgrenades::isflashbanged() )');
// Profile synchronous sections only: none of these functions waits.
for (const [name, args, slot] of [['target_loop', '', 0], ['aim_loop', '', 1], ['doBotMovement_loop', 'data', 2]]) {
    internal = replace(internal, `${name}(${args ? ' ' + args + ' ' : ''})\n{`,
        `${name}_script(${args ? ' ' + args + ' ' : ''})\n{`);
    internal += `\n${name}( ${args} )\n{\n\tif ( isdefined( level.native_profile ) && level.native_profile )\n\t\tbotnativeprofilebegin( ${slot} );\n\tself ${name}_script( ${args} );\n\tif ( isdefined( level.native_profile ) && level.native_profile )\n\t\tbotnativeprofileend( ${slot} );\n}\n`;
}
write(internalPath, internal);
// Remove the active AI thread tree in native mode; leave the old hybrid available for a map-reload A/B.
internal = replace(internal, '\tself thread onUAVUpdate();',
    '\tif ( !level.native_controller )\n\t\tself thread onUAVUpdate();');
internal = replace(internal, '\t\tself resetBotVars();\n\t\tself thread onWeaponChange();',
    '\t\tself resetBotVars();\n\t\tif ( level.native_controller )\n\t\t{\n\t\t\tself botnativecontrol( true );\n\t\t\tself thread BotNativeDeath();\n\t\t\tcontinue;\n\t\t}\n\t\tself thread onWeaponChange();');
internal += `
BotNativeDeath()
{
	self endon( "disconnect" );
	self waittill( "death" );
	// Clear combat state while retaining native input ownership for respawn requests.
	self botnativecontrol( true );
}
`;
write(internalPath, internal);
const scriptPath = 'maps/mp/bots/_bot_script.gsc';
let script = read(scriptPath);
script = replace(script, '\tself thread difficulty();\n\tself thread teamWatch();',
    '\tif ( !level.native_controller )\n\t\tself thread difficulty();\n\tself thread teamWatch();');
script = replace(script, '\tself thread onBotSpawned();\n\tself thread onSpawned();\n\tself thread onDeath();\n\tself thread onKillcam();',
    '\tself thread onDeath();\n\tif ( !level.native_controller )\n\t{\n\t\tself thread onBotSpawned();\n\t\tself thread onSpawned();\n\t\tself thread onKillcam();\n\t}');
// Team/class watchers only maintain menus; their 20 Hz polling is unnecessary for this experiment.
for (const fn of ['teamWatch', 'classWatch']) {
    const start = script.indexOf('\n' + fn + '()\n{');
    const end = script.indexOf('\n}\n', start) + 3;
    if (start < 0 || end < start) throw new Error('Missing menu watcher: ' + fn);
    let body = script.slice(start, end);
    body = body.replace('\n{', '\n{\n\tmenu_wait = 0.05;\n\tif ( level.native_controller )\n\t\tmenu_wait = 0.25;');
    body = body.replaceAll('wait .05;', 'wait menu_wait;');
    script = script.slice(0, start) + body + script.slice(end);
}
write(scriptPath, script);
const mainPath = 'maps/mp/bots/_bot.gsc';
let main = read(mainPath);
main = replace(main, '\t\tplayer thread onGrenadeFire();\n\t\tplayer thread onWeaponFired();',
    '\t\tif ( !level.native_controller )\n\t\t{\n\t\t\tplayer thread onGrenadeFire();\n\t\t\tplayer thread onWeaponFired();\n\t\t}');
write(mainPath, main);
fs.writeFileSync(path.join(output, 'native-bots.cfg'), `// Execute BEFORE loading a map. Defaults for the native controller experiment.\r\nset bots_native_controller 1\r\nset bots_native_drive 1\r\nset bots_native_profile 1\r\nset bots_native_sight_budget 6\r\nset bots_native_path_budget 1\r\nset bots_native_reaction_ms 250\r\nset bots_native_range 4000\r\nset bots_native_fov 180\r\nset bots_native_turn_rate 300\r\nset bots_native_path 1\r\nset bots_native_aim 1\r\nset bots_native_sight_prefilter 1\r\nset bots_native_sight_order 1\r\n// bots_main_awareness_quality is unused by the native controller.\r\n// Set bots_native_controller 0 before reloading for the prior hybrid implementation.\r\n`);
fs.writeFileSync(path.join(root, 'build/iw3-native-bots/source-manifest.json'), JSON.stringify({source, hashes}, null, 2));
console.log('Prepared full experimental mod: ' + output);
