#include "platform/xbox360.h"

#include "platform/platform.h"

namespace codxe
{
namespace
{

const uint32_t kXexpFinishExecutableLoadRetail = 0x8007AF68;
const uint32_t kXexpFinishExecutableLoadDevkit = 0x800A17C8;

} // namespace

void *ResolveExport(const char *module_name, unsigned int ordinal)
{
    HMODULE module_handle = GetModuleHandle(module_name);
    if (module_handle == 0)
        return 0;

    return GetProcAddress(module_handle, reinterpret_cast<const char *>(ordinal));
}

uint32_t GetCurrentTitleId()
{
    XexExecutionId *execution_id = 0;
    if (XamGetExecutionId(&execution_id) == 0 && execution_id != 0)
        return static_cast<uint32_t>(execution_id->title.title_id);

    return static_cast<uint32_t>(XamGetCurrentTitleId());
}

uint32_t GetCurrentExecutableTimestamp()
{
    LoaderDataTableEntry *module = reinterpret_cast<LoaderDataTableEntry *>(GetModuleHandle(0));
    return GetExecutableTimestamp(module);
}

uint32_t GetExecutableTimestamp(const LoaderDataTableEntry *module)
{
    return module != 0 ? static_cast<uint32_t>(module->time_date_stamp) : 0;
}

XexpFinishExecutableLoad_t GetXexpFinishExecutableLoad(PlatformKind platform)
{
    if (platform == PLATFORM_XBOX360_DEVKIT)
        return reinterpret_cast<XexpFinishExecutableLoad_t>(kXexpFinishExecutableLoadDevkit);

    if (platform == PLATFORM_XBOX360_RETAIL)
        return reinterpret_cast<XexpFinishExecutableLoad_t>(kXexpFinishExecutableLoadRetail);

    return 0;
}

} // namespace codxe
