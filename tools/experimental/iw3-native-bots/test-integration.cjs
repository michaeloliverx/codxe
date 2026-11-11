const assert = require('assert/strict');
const fs = require('fs');
const path = require('path');
const crypto = require('crypto');
const root = path.resolve(__dirname, '../../..');
const build = path.join(root, 'build/iw3-native-bots');
const manifest = JSON.parse(fs.readFileSync(path.join(build, 'source-manifest.json')));
const hash = p => crypto.createHash('sha256').update(fs.readFileSync(p)).digest('hex');
const changed = new Set(Object.keys(manifest.hashes));
let unchanged = 0;
for (const [relative, expected] of Object.entries(manifest.hashes))
    assert.equal(hash(path.join(manifest.source, relative)), expected, 'Source mod was modified');
function walk(relative) {
    for (const entry of fs.readdirSync(path.join(manifest.source, relative), {withFileTypes: true})) {
        const child = path.join(relative, entry.name);
        if (entry.isDirectory()) walk(child);
        else if (!changed.has(child.replaceAll('\\', '/'))) {
            assert.equal(hash(path.join(manifest.source, child)), hash(path.join(build, 'mod', child)), child);
            ++unchanged;
        }
    }
}
for (const dir of ['maps', 'scripts', 'scriptdata']) walk(dir);
const internal = fs.readFileSync(path.join(build, 'mod/maps/mp/bots/_bot_internal.gsc'), 'utf8');
const utility = fs.readFileSync(path.join(build, 'mod/maps/mp/bots/_bot_utility.gsc'), 'utf8');
const script = fs.readFileSync(path.join(build, 'mod/maps/mp/bots/_bot_script.gsc'), 'utf8');
const main = fs.readFileSync(path.join(build, 'mod/maps/mp/bots/_bot.gsc'), 'utf8');
const registry = fs.readFileSync(path.join(root, 'src/game/iw3/mp/components/gsc.cpp'), 'utf8');
const builtins = new Set([...registry.matchAll(/\{"(botnav\w+|botnative\w+)"/g)].map(m => m[1]));
for (const match of (internal + utility + script + main).matchAll(/\b(botnav\w+|botnative\w+)\s*\(/g))
    assert(builtins.has(match[1]), 'Unregistered builtin: ' + match[1]);
assert(utility.includes('setdvar( "bots_native_sight_prefilter", 1 )'));
assert(utility.includes('setdvar( "bots_native_sight_order", 1 )'));
assert(internal.includes('if ( !sighttracepassed( myEye, boneLoc, false, undefined ) )\r\n\t\t\treturn false;'));
assert(utility.includes('!getdvarint( "bots_native_path" ) && !level.native_controller'));
assert(internal.includes('self botnativeaimstop();'));
// Native input must have exclusive ownership: do not start either legacy AI thread tree.
assert(internal.includes('self thread BotNativeDeath();\r\n\t\t\tcontinue;\r\n\t\t}\r\n\t\tself thread onWeaponChange();'));
assert(script.includes('if ( !level.native_controller )\r\n\t{\r\n\t\tself thread onBotSpawned();'));
assert(main.includes('if ( !level.native_controller )\r\n\t\t{\r\n\t\t\tplayer thread onGrenadeFire();'));
assert(utility.includes('setdvar( "bots_native_controller", 1 )'));
assert(utility.includes('botnavnode( i, wp.origin, extra, traversal )'));
const profileReport = utility.slice(utility.indexOf('BotNativeProfileReport()\r\n{'));
assert(profileReport.includes('botnativeprofilestats();'));
assert(!profileReport.includes('BotBuiltinPrintConsole('), 'Profiler output must not rely on GSC println');

// Exhaust all predicate values, including previously-seen and forcibly assigned targets.
for (let bits = 0; bits < 128; ++bits) {
    const [head, left, right, smoke, near, fov, forced] = Array.from({length: 7}, (_, i) => !!(bits & 1 << i));
    const original = ((head || left || right) && (smoke || near) && fov) || forced;
    const prefiltered = (fov && (smoke || near) && (head || left || right)) || forced;
    assert.equal(prefiltered, original);
}
// Sight, clear bullet path, and glass acceptance are independent inputs to the original bone predicate.
for (let bits = 0; bits < 8; ++bits) {
    const sight = !!(bits & 1), clear = !!(bits & 2), glass = !!(bits & 4);
    let bulletCalls = 0;
    function bullet() { ++bulletCalls; return clear || glass; }
    const original = bullet() && sight;
    assert.equal(bulletCalls, 1);
    bulletCalls = 0;
    const reordered = sight && bullet();
    assert.equal(reordered, original);
    assert.equal(bulletCalls, sight ? 1 : 0);
}
let state = 123456789;
function random() { state = (Math.imul(state, 1664525) + 1013904223) >>> 0; return state; }
let originalTraces = 0, optimizedTraces = 0;
for (let test = 0; test < 10000; ++test) {
    const distances = Array.from({length: 128}, () => random() % 32); // Many ties.
    const visible = distances.map(() => random() % 3 === 0);
    let original = -1, optimized = -1, oldDistance = 2147483647, newDistance = 2147483647;
    for (let i = distances.length - 1; i >= 0; --i) {
        ++originalTraces;
        if (visible[i] && distances[i] <= oldDistance) { oldDistance = distances[i]; original = i; }
        if (distances[i] > newDistance) continue;
        ++optimizedTraces;
        if (visible[i]) { newDistance = distances[i]; optimized = i; }
    }
    assert.equal(optimized, original);
}
console.log(`PASS: ${unchanged} unmodified mod files byte-identical; input scripts unchanged; native builtins registered`);
console.log('PASS: 128 candidate predicates, 8 bone sight/bullet/glass predicates, and 10,000 nearest-visible results');
console.log(`Synthetic nearest-visible traces: ${originalTraces} -> ${optimizedTraces} (algorithm check, not game performance)`);
