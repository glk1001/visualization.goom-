#ifndef _GOOM_TESTING_H
#define _GOOM_TESTING_H

#define GOOM_VER_NAME        "Goom Library"
#define GOOM_VER_DESCRIPTION "Added goom logging"  
#define GOOM_VER_MAJOR       1
#define GOOM_VER_MINOR       0
#define GOOM_VER_REVISION    3

#define GOOM_FULL_VERSION_STR GOOMVER_NAME " " \
                              GOOM_VER_MAJOR "." GOOM_VER_MINOR "." GOOM_VER_REVISION ", " \
                              GOOM_VER_DESCRIPTION
                              
extern void getVersionAtRuntime(int* verMajor, int* verMinor, int* verRevision);
extern const char* getVersionDescriptionAtRuntime();

#endif
