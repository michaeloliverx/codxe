#include "core/target.h"

#include "games/iw3_mp/target.h"

#include <cstddef>

namespace codxe
{
namespace
{

const TargetDef *const kTargets[] = {
    &games::iw3_mp::kTarget,
};

} // namespace

const TargetDef *FindTarget(uint32_t title_id, uint32_t timestamp)
{
    for (size_t i = 0; i < sizeof(kTargets) / sizeof(kTargets[0]); ++i)
    {
        const TargetDef *target = kTargets[i];
        if (target->title_id == title_id && target->timestamp == timestamp)
            return target;
    }

    return 0;
}

} // namespace codxe
