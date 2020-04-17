#include "goom_testing.h"

void getVersionAtRuntime(int* verMajor, int* verMinor, int* verRevision)
{
    *verMajor = GOOM_VER_MAJOR;
    *verMinor = GOOM_VER_MINOR;
    *verRevision = GOOM_VER_REVISION;
}

const char* getVersionDescriptionAtRuntime()
{
    return GOOM_VER_DESCRIPTION;
}
