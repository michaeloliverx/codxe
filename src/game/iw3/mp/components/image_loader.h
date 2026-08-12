#pragma once

#include "pch.h"

namespace iw3
{
namespace mp
{
class image_loader : public Module
{

  public:
    image_loader();
    ~image_loader();

    static void OnAssetLink(XAsset *asset);
    static void OnDelayStreamLoad();
};
} // namespace mp
} // namespace iw3
