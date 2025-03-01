namespace mp
{
    struct MapEnts
    {
        const char *name;
        char *entityString;
        int numEntityChars;
    };

    typedef void (*Load_MapEntsPtr_t)();

}