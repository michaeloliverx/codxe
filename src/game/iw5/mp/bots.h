#pragma once

#include "pch.h"
namespace iw5
{
namespace mp
{

    void SpawnBot(scr_entref_t entref);

    class Bots : public Module
    {
      public:
        Bots();
        ~Bots();
    };
} // namespace mp
} // namespace iw5