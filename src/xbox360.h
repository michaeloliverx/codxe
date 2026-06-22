#pragma once

namespace xbox
{

enum Environment
{
    ENVIRONMENT_XENIA,
    ENVIRONMENT_XBOX_RETAIL,
    ENVIRONMENT_XBOX_DEVKIT,
};

Environment GetEnvironment();
const char *GetEnvironmentName(Environment environment);
void ApplySystemPatches();
void Notify(const char *message);

} // namespace xbox
