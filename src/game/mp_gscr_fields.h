#pragma once

#include "common.h"

namespace mp
{

    enum fieldtype_t
    {
        F_INT = 0x0,
        F_FLOAT = 0x1,
        F_LSTRING = 0x2,
        F_STRING = 0x3,
        F_VECTOR = 0x4,
        F_ENTITY = 0x5,
        F_ENTHANDLE = 0x6,
        F_VECTORHACK = 0x7,
        F_OBJECT = 0x8,
        F_MODEL = 0x9,
    };

    struct client_fields_s
    {
        const char *name;
        int ofs;
        fieldtype_t type;
        void (*setter)(gclient_s *, const client_fields_s *);
        void (*getter)(gclient_s *, const client_fields_s *);
    };

    struct ent_field_t
    {
        const char *name;
        int ofs;
        fieldtype_t type;
        void (*callback)(gentity_s *, int);
    };

    typedef void (*ClientScr_ReadOnly_t)(gclient_s *client, const client_fields_s *field);
    typedef void (*ClientScr_GetName_t)(gclient_s *client, const client_fields_s *field);
    typedef void (*ClientScr_SetSessionTeam_t)(gclient_s *client, const client_fields_s *field);
    typedef void (*ClientScr_GetSessionTeam_t)(gclient_s *client, const client_fields_s *field);
    typedef void (*ClientScr_SetSessionState_t)(gclient_s *client, const client_fields_s *field);
    typedef void (*ClientScr_GetSessionState_t)(gclient_s *client, const client_fields_s *field);
    typedef void (*ClientScr_SetMaxHealth_t)(gclient_s *client, const client_fields_s *field);
    typedef void (*ClientScr_SetScore_t)(gclient_s *client, const client_fields_s *field);
    typedef void (*ClientScr_SetStatusIcon_t)(gclient_s *client, const client_fields_s *field);
    typedef void (*ClientScr_GetStatusIcon_t)(gclient_s *client, const client_fields_s *field);
    typedef void (*ClientScr_SetHeadIcon_t)(gclient_s *client, const client_fields_s *field);
    typedef void (*ClientScr_GetHeadIcon_t)(gclient_s *client, const client_fields_s *field);
    typedef void (*ClientScr_SetHeadIconTeam_t)(gclient_s *client, const client_fields_s *field);
    typedef void (*ClientScr_GetHeadIconTeam_t)(gclient_s *client, const client_fields_s *field);
    typedef void (*ClientScr_SetSpectatorClient_t)(gclient_s *client, const client_fields_s *field);
    typedef void (*ClientScr_SetKillCamEntity_t)(gclient_s *client, const client_fields_s *field);
    typedef void (*ClientScr_SetArchiveTime_t)(gclient_s *client, const client_fields_s *field);
    typedef void (*ClientScr_GetArchiveTime_t)(gclient_s *client, const client_fields_s *field);
    typedef void (*ClientScr_SetPSOffsetTime_t)(gclient_s *client, const client_fields_s *field);
    typedef void (*ClientScr_GetPSOffsetTime_t)(gclient_s *client, const client_fields_s *field);

    typedef void (*GScr_AddFieldsForClient_t)();
    typedef void (*Scr_AddClassField_t)(unsigned int classnum, const char *name, unsigned __int16 offset);

    typedef int (*Scr_SetEntityField_t)(int entnum, int offset);
    typedef void (*Scr_GetEntityField_t)(int entnum, int offset);

    typedef void (*Scr_SetGenericField_t)(unsigned __int8 *b, fieldtype_t type, int ofs);
    typedef void (*Scr_GetGenericField_t)(unsigned __int8 *b, fieldtype_t type, int ofs);

    void init_gscr_fields();
}