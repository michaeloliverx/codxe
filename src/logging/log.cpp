#include "logging/log.h"

#include "core/context.h"
#include "platform/xbox360.h"

#include <cstdarg>
#include <cstdio>

namespace codxe
{
namespace
{

const char *GetLogLevelName(LogLevel level)
{
    switch (level)
    {
    case LOG_ERROR:
        return "error";
    case LOG_WARN:
        return "warn";
    case LOG_INFO:
        return "info";
    case LOG_DEBUG:
        return "debug";
    default:
        return "unknown";
    }
}

bool ShouldLog(const LogState *state, LogLevel level)
{
    if (state == 0)
        return true;

    return level <= state->min_level;
}

} // namespace

void LogInit(LogState *state)
{
    if (state == 0)
        return;

    state->min_level = LOG_INFO;
}

void Log(PluginContext *ctx, LogLevel level, const char *format, ...)
{
    if (format == 0)
        return;

    const LogState *state = ctx != 0 ? &ctx->log : 0;
    if (!ShouldLog(state, level))
        return;

    char message[512];

    va_list args;
    va_start(args, format);
    _vsnprintf_s(message, sizeof(message), _TRUNCATE, format, args);
    va_end(args);

    message[sizeof(message) - 1] = '\0';

    DbgPrint("[codxe][%s] %s\n", GetLogLevelName(level), message);
}

} // namespace codxe
