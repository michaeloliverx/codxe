#pragma once

namespace xbox
{
namespace notifications
{
void Initialize(const char *message = "Loaded");
void Shutdown();
void OnTitleTerminate();
} // namespace notifications
} // namespace xbox
