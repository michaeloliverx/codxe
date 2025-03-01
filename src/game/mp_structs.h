namespace mp
{
    struct MapEnts
    {
        const char *name;
        char *entityString;
        int numEntityChars;
    };

    typedef void (*Load_MapEntsPtr_t)();
    typedef char *(*Scr_ReadFile_FastFile_t)(const char *filename, const char *extFilename, const char *codePos, bool archive);
}
