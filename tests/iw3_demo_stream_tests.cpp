#define _CRT_SECURE_NO_WARNINGS
#include "../src/game/iw3/mp/components/demo_stream.h"
#include <assert.h>
#include <math.h>

using namespace iw3_demo;

namespace
{
Header Layout()
{
    Header result = {};
    result.gameStateSize = 4;
    result.playerStateSize = 4;
    result.entityStateSize = 4;
    result.clientStateSize = 4;
    result.snapshotPrefixSize = 12;
    result.commandSequence = 5;
    strcpy(result.map, "mp_test");
    return result;
}

FILE *Fixture()
{
    FILE *file = fopen("build/validation/iw3-demo/stream-test.tmp", "w+b");
    assert(file);
    assert(WriteHeader(file, Layout()));
    assert(WriteUInt(file, 0));
    return file;
}

void Payload(FILE *file, uint32_t type, uint32_t time, uint32_t number = 0)
{
    const uint32_t size = type == SNAPSHOT ? 16 : type == CAMERA ? 4 : 1;
    assert(WriteRecord(file, type, size, time, number));
    char zeros[16] = {};
    assert(Write(file, zeros, size));
}

void Frames(FILE *file)
{
    Payload(file, SNAPSHOT, 100);
    Payload(file, CAMERA, 90);
    Payload(file, CAMERA, 106);
    Payload(file, SNAPSHOT, 150);
    Payload(file, CAMERA, 155);
}

bool Load(FILE *file, Index &index)
{
    rewind(file);
    Header header;
    if (!ReadHeader(file, header, Layout()))
        return false;
    uint32_t state;
    return ReadUInt(file, state) && Scan(file, header, index);
}

void HeaderAndTimeline()
{
    FILE *file = Fixture();
    Frames(file);
    assert(WriteRecord(file, END, 0, 155));
    Index index;
    assert(Load(file, index));
    assert(index.complete && index.snapshots.size() == 2 && index.cameras.size() == 3);
    assert(Find(index.cameras, -1) == 0);
    assert(Find(index.cameras, 100) == 0);
    assert(Find(index.cameras, 106) == 1);
    assert(Find(index.cameras, 106.5) == 1);
    assert(Find(index.cameras, 10000) == 2);
    rewind(file);
    char bytes[12];
    assert(Read(file, bytes, sizeof(bytes)));
    assert(memcmp(bytes, "IW3XET4\0\1\0\0\0", sizeof(bytes)) == 0);
    fclose(file);
}

void RecoveryAndRejection()
{
    Index index;
    FILE *file = Fixture();
    Frames(file);
    assert(Load(file, index) && !index.complete);
    assert(fseek(file, 0, SEEK_END) == 0);
    assert(WriteRecord(file, CAMERA, 4, 160));
    assert(Write(file, "x", 1));
    assert(Load(file, index) && !index.complete && index.cameras.size() == 3);
    fclose(file);

    file = Fixture();
    Frames(file);
    assert(WriteRecord(file, 99, 0, 160));
    assert(!Load(file, index));
    fclose(file);

    file = Fixture();
    Frames(file);
    Payload(file, CAMERA, 155); // Duplicate camera times cannot form an interpolation interval.
    assert(!Load(file, index));
    fclose(file);

    file = Fixture();
    Frames(file);
    assert(WriteRecord(file, CAMERA, 3, 160));
    assert(Write(file, "bad", 3));
    assert(!Load(file, index));
    fclose(file);

    file = Fixture();
    Payload(file, SNAPSHOT, 100);
    assert(WriteRecord(file, END, 0, 100));
    assert(!Load(file, index));
    fclose(file);

    file = Fixture();
    Frames(file);
    assert(fseek(file, 8, SEEK_SET) == 0);
    assert(WriteUInt(file, 2)); // Incompatible format version.
    assert(!Load(file, index));
    assert(fseek(file, 8, SEEK_SET) == 0);
    assert(WriteUInt(file, 1));
    assert(WriteUInt(file, 0xFFFFFFFF)); // Incompatible game-state layout.
    assert(!Load(file, index));
    fclose(file);
}

void Commands()
{
    FILE *file = Fixture();
    for (uint32_t i = 6; i < 206; ++i)
        Payload(file, COMMAND, 100, i);
    Frames(file);
    assert(WriteRecord(file, END, 0, 155));
    Index index;
    assert(Load(file, index) && index.commands.size() == 200);
    assert(index.commands.front().number == 6 && index.commands.back().number == 205);
    fclose(file);

    file = Fixture();
    Frames(file);
    Payload(file, COMMAND, 100, 7); // Missing command 6.
    assert(!Load(file, index));
    fclose(file);
}

void PlaybackClock()
{
    Clock clock;
    clock.Reset(1000, 100);
    for (uint32_t i = 101; i <= 200; ++i)
        clock.Tick(i, 0.001f, 10000, 0);
    assert(fabs(clock.time - 1000.1) < 0.00001);
    clock.paused = true;
    clock.Tick(5000, 1, 10000, 0);
    assert(fabs(clock.time - 1000.1) < 0.00001);
    clock.paused = false;
    clock.Tick(5010, 1, 10000, 0);
    assert(fabs(clock.time - 1010.1) < 0.00001); // No catch-up jump after a pause.
    clock.Reset(0, 0xFFFFFFF0);
    clock.Tick(0x10, 1, 10000, 0);
    assert(clock.time == 32); // Millisecond counter wrap.
    clock.Reset(0, 1);
    clock.Tick(1000, 0.5f, 10000, 4);
    clock.Tick(9000, 0.5f, 10000, 4);
    assert(clock.time == 4); // Capture stepping ignores disk/render stalls.
    clock.Tick(9001, 16, 5, 4);
    assert(clock.time == 5 && clock.paused && clock.finished);
    clock.Tick(9002, 1, 5, 0);
    assert(clock.time == 5 && clock.paused && clock.finished); // Holding the last frame remains stable.
    clock.Reset(2, 9002);
    assert(clock.paused && !clock.finished); // Seeking/restarting clears completion but preserves pause.
    clock.Reset(5, 9002);
    clock.Tick(9003, 1, 5, 0);
    assert(clock.finished); // Seeking to the end also completes a paused demo.
    clock.paused = false;
    clock.Reset(0, 0);
    for (uint32_t frame = 0; frame < 60; ++frame)
        clock.Tick(frame * 123, 0.001f, 10000, 1000.0 / 60);
    assert(fabs(clock.time - 1.0) < 0.00001); // Fractional 60 fps capture steps at 0.001x.
    assert(fabs(LerpAngle(179, -179, 0.5f) - 180) < 0.001);
    assert(fabs(LerpAngle(-179, 179, 0.5f) + 180) < 0.001);
}
} // namespace

int main()
{
    HeaderAndTimeline();
    RecoveryAndRejection();
    Commands();
    PlaybackClock();
    assert(remove("build/validation/iw3-demo/stream-test.tmp") == 0);
    puts("IW3 demo stream/timeline tests passed.");
    return 0;
}
