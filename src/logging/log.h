#pragma once

namespace codxe
{

struct PluginContext;

enum LogLevel
{
    LOG_ERROR,
    LOG_WARN,
    LOG_INFO,
    LOG_DEBUG,
};

struct LogState
{
    LogLevel min_level;
};

void LogInit(LogState *state);
void Log(PluginContext *ctx, LogLevel level, const char *format, ...);

} // namespace codxe
