namespace sp
{
    typedef char *(*Scr_AddSourceBuffer_t)(const char *filename, const char *extFilename, const char *codePos, bool archive);
    typedef char *(*Scr_ReadFile_FastFile_t)(const char *filename, const char *extFilename, const char *codePos, bool archive);
}
