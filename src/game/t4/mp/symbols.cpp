#include "common.h"
#include "symbols.h"

namespace t4
{
    namespace mp
    {
        clipMap_t *cm = reinterpret_cast<clipMap_t *>(0x82DD4F80);
        client_fields_s *client_fields = reinterpret_cast<client_fields_s *>(0x82026B80);
        gentity_s *g_entities = reinterpret_cast<gentity_s *>(0x82BAD1B0);
    }
}
