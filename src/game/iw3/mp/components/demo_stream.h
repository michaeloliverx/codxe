#pragma once

// The envelope is little endian; game-state payloads retain the TU4 Xbox layouts.
// Keep this independent of the game/SDK so damaged recordings can be tested on a PC.
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <vector>

namespace iw3_demo
{
enum RecordType
{
    SNAPSHOT = 1,
    CAMERA = 2,
    COMMAND = 3,
    END = 4
};

struct Header
{
    uint32_t gameStateSize;
    uint32_t playerStateSize;
    uint32_t entityStateSize;
    uint32_t clientStateSize;
    uint32_t snapshotPrefixSize;
    uint32_t clientNum;
    uint32_t commandSequence;
    char map[64];
    char mod[64];
};

struct Record
{
    uint32_t type;
    uint32_t size;
    uint32_t time;
    uint32_t number;
    long offset;
};

inline bool Read(FILE *file, void *data, size_t size)
{
    return fread(data, 1, size, file) == size;
}

inline bool Write(FILE *file, const void *data, size_t size)
{
    const long position = ftell(file);
    return position >= 0 && size <= static_cast<size_t>(INT_MAX - position) && fwrite(data, 1, size, file) == size;
}

inline bool WriteUInt(FILE *file, uint32_t value)
{
    const unsigned char bytes[4] = {static_cast<unsigned char>(value), static_cast<unsigned char>(value >> 8),
                                    static_cast<unsigned char>(value >> 16), static_cast<unsigned char>(value >> 24)};
    return Write(file, bytes, sizeof(bytes));
}

inline bool ReadUInt(FILE *file, uint32_t &value)
{
    unsigned char bytes[4];
    if (!Read(file, bytes, sizeof(bytes)))
        return false;
    value = bytes[0] | (static_cast<uint32_t>(bytes[1]) << 8) | (static_cast<uint32_t>(bytes[2]) << 16) |
            (static_cast<uint32_t>(bytes[3]) << 24);
    return true;
}

inline bool WriteHeader(FILE *file, const Header &header)
{
    return Write(file, "IW3XET4", 8) && WriteUInt(file, 1) && WriteUInt(file, header.gameStateSize) &&
           WriteUInt(file, header.playerStateSize) && WriteUInt(file, header.entityStateSize) &&
           WriteUInt(file, header.clientStateSize) && WriteUInt(file, header.snapshotPrefixSize) &&
           WriteUInt(file, header.clientNum) && WriteUInt(file, header.commandSequence) &&
           Write(file, header.map, sizeof(header.map)) && Write(file, header.mod, sizeof(header.mod));
}

inline bool ReadHeader(FILE *file, Header &header, const Header &layout)
{
    char magic[8];
    uint32_t version;
    return Read(file, magic, sizeof(magic)) && memcmp(magic, "IW3XET4", 8) == 0 && ReadUInt(file, version) &&
           version == 1 && ReadUInt(file, header.gameStateSize) && ReadUInt(file, header.playerStateSize) &&
           ReadUInt(file, header.entityStateSize) && ReadUInt(file, header.clientStateSize) &&
           ReadUInt(file, header.snapshotPrefixSize) && ReadUInt(file, header.clientNum) &&
           ReadUInt(file, header.commandSequence) && Read(file, header.map, sizeof(header.map)) &&
           Read(file, header.mod, sizeof(header.mod)) && header.gameStateSize == layout.gameStateSize &&
           header.playerStateSize == layout.playerStateSize && header.entityStateSize == layout.entityStateSize &&
           header.clientStateSize == layout.clientStateSize && header.snapshotPrefixSize == layout.snapshotPrefixSize &&
           header.clientNum < 24 && header.commandSequence < INT_MAX && memchr(header.map, 0, sizeof(header.map)) &&
           header.map[0] && memchr(header.mod, 0, sizeof(header.mod));
}

inline bool WriteRecord(FILE *file, uint32_t type, uint32_t size, uint32_t time, uint32_t number = 0)
{
    return WriteUInt(file, type) && WriteUInt(file, size) && WriteUInt(file, time) && WriteUInt(file, number);
}

struct Index
{
    std::vector<Record> snapshots;
    std::vector<Record> cameras;
    std::vector<Record> commands;
    bool complete;

    Index() : complete(false)
    {
    }
};

// A cut-off final write is recoverable. Invalid envelopes and discontinuous commands are not.
inline bool Scan(FILE *file, const Header &header, Index &index)
{
    index = Index();
    const long start = ftell(file);
    if (start < 0 || fseek(file, 0, SEEK_END) != 0)
        return false;
    const long end = ftell(file);
    if (end < start || fseek(file, start, SEEK_SET) != 0)
        return false;

    uint32_t command = header.commandSequence;
    while (ftell(file) <= end - 16)
    {
        Record record;
        if (!ReadUInt(file, record.type) || !ReadUInt(file, record.size) || !ReadUInt(file, record.time) ||
            !ReadUInt(file, record.number))
            return false;
        record.offset = ftell(file);
        if (record.time > INT_MAX || record.size > static_cast<uint32_t>(end - record.offset))
            break;
        std::vector<Record> *records;
        switch (record.type)
        {
        case SNAPSHOT:
            if (record.size < header.snapshotPrefixSize + 4 || record.size > header.snapshotPrefixSize +
                                                                                 512 * header.entityStateSize +
                                                                                 24 * header.clientStateSize + 4)
                return false;
            records = &index.snapshots;
            break;
        case CAMERA:
            if (record.size != header.playerStateSize)
                return false;
            records = &index.cameras;
            break;
        case COMMAND:
            if (record.size < 1 || record.size > 1024 || record.number >= INT_MAX || record.number != command + 1)
                return false;
            command = record.number;
            records = &index.commands;
            break;
        case END:
            index.complete = record.size == 0 && record.offset == end;
            return index.complete && index.snapshots.size() >= 2 && index.cameras.size() >= 2;
        default:
            return false;
        }
        if (records->size() >= 1000000 ||
            (!records->empty() &&
             (record.time < records->back().time || (record.type != COMMAND && record.time == records->back().time))))
            return false;
        records->push_back(record);
        if (fseek(file, static_cast<long>(record.size), SEEK_CUR) != 0)
            return false;
    }
    return index.snapshots.size() >= 2 && index.cameras.size() >= 2;
}

// Last record at/before time; clamped to the first sample before recording begins.
inline size_t Find(const std::vector<Record> &records, double time)
{
    size_t first = 0;
    size_t last = records.size();
    while (first < last)
    {
        const size_t middle = first + (last - first) / 2;
        if (records[middle].time <= time)
            first = middle + 1;
        else
            last = middle;
    }
    return first ? first - 1 : 0;
}

inline float LerpAngle(float from, float to, float fraction)
{
    float delta = to - from;
    while (delta > 180.0f)
        delta -= 360.0f;
    while (delta < -180.0f)
        delta += 360.0f;
    return from + delta * fraction;
}

class Clock
{
  public:
    double time;
    bool paused;
    bool finished;

    Clock() : time(0.0), paused(false), finished(false), previous(0)
    {
    }

    void Reset(double value, uint32_t now)
    {
        time = value;
        finished = false;
        previous = now;
    }

    void Tick(uint32_t now, float rate, double end, double fixedMsec)
    {
        const uint32_t elapsed = now - previous;
        previous = now;
        if (!paused)
        {
            // Preserve sub-millisecond time at low rates. fixedMsec advances once per rendered frame.
            time += (fixedMsec > 0 ? fixedMsec : elapsed) * static_cast<double>(rate);
        }
        if (time >= end)
        {
            time = end;
            paused = true;
            finished = true;
        }
    }

  private:
    uint32_t previous;
};
} // namespace iw3_demo
