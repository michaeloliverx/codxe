#include "../../common.h"

namespace t4
{
    struct InitCheck
    {
        const char *address;
        const char *match;
        const wchar_t *message;
        void (*init)();
    };

    static const InitCheck checks[] = {
        {reinterpret_cast<const char *>(0x820024CC), "multiplayer", L"T4 mp", nullptr},
        {reinterpret_cast<const char *>(0x820024EC), "multiplayer", L"T4 TU0 detected. Please install the TU7 version of the game.", nullptr},
    };

    void init()
    {
        for (size_t i = 0; i < sizeof(checks) / sizeof(checks[0]); i++)
        {
            const InitCheck &check = checks[i];
            if (std::strncmp(check.address, check.match, std::strlen(check.match)) == 0)
            {
                xbox::show_notification(check.message);

                if (check.init)
                    check.init(); // only call if it's non-null

                return;
            }
        }

        xbox::show_notification(L"T4 unsupported executable");
    }
}
