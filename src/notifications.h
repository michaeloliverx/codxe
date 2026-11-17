#pragma once

#include "xbox360.h"

namespace xbox
{
namespace notifications
{
void Initialize(Environment environment, const char *message = "Loaded");
void Shutdown();
void OnTitleTerminate();
} // namespace notifications
} // namespace xbox
