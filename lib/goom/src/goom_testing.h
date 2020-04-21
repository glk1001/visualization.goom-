#ifndef _GOOM_TESTING_H
#define _GOOM_TESTING_H

#include <stdio.h>

#define GOOM_VER_NAME "Goom Library"
#define GOOM_VER_DESCRIPTION "Fixing gcc warnings"
#define GOOM_VER_MAJOR 1
#define GOOM_VER_MINOR 0
#define GOOM_VER_REVISION 13

#define GOOM_FULL_VERSION_STR                                                                      \
  GOOMVER_NAME " " GOOM_VER_MAJOR "." GOOM_VER_MINOR "." GOOM_VER_REVISION ","                     \
                                                                           " " GOOM_VER_DESCRIPTION

extern void getVersionAtRuntime(int* verMajor, int* verMinor, int* verRevision);
extern const char* getVersionDescriptionAtRuntime();

extern int getIntSetting(FILE* f, const char* parent, const char* settingName,
                         const char* srceFilename, int lineNum);
static void saveIntSetting(FILE* f, const char* parent, const char* settingName, int value,
                           const char* srceFilename, int lineNum);
extern float getFloatSetting(FILE* f, const char* parent, const char* settingName,
                             const char* srceFilename, int lineNum);
static void saveFloatSetting(FILE* f, const char* parent, const char* settingName, float value,
                             const char* srceFilename, int lineNum);
extern int getIndexedIntSetting(FILE* f, const char* parent, const char* settingName, int i,
                                const char* srceFilename, int lineNum);
static void saveIndexedIntSetting(FILE* f, const char* parent, const char* settingName, int i,
                                  int value, const char* srceFilename, int lineNum);
extern float getIndexedFloatSetting(FILE* f, const char* parent, const char* settingName, int i,
                                    const char* srceFilename, int lineNum);
static void saveIndexedFloatSetting(FILE* f, const char* parent, const char* settingName, int i,
                                    float value, const char* srceFilename, int lineNum);

#define get_int_setting(f, p, s) getIntSetting(f, p, s, __FILE__, __LINE__)
#define save_int_setting(f, p, s, v) saveIntSetting(f, p, s, v, __FILE__, __LINE__)
#define get_float_setting(f, p, s) getFloatSetting(f, p, s, __FILE__, __LINE__)
#define save_float_setting(f, p, s, v) saveFloatSetting(f, p, s, v, __FILE__, __LINE__)
#define get_indexed_int_setting(f, p, s, i) getIndexedIntSetting(f, p, s, i, __FILE__, __LINE__)
#define save_indexed_int_setting(f, p, s, i, v)                                                    \
  saveIndexedIntSetting(f, p, s, i, v, __FILE__, __LINE__)
#define get_indexed_float_setting(f, p, s, i) getIndexedFloatSetting(f, p, s, i, __FILE__, __LINE__)
#define save_indexed_float_setting(f, p, s, i, v)                                                  \
  saveIndexedFloatSetting(f, p, s, i, v, __FILE__, __LINE__)

#define UNUSED(x) (void)x

static inline void saveIntSetting(FILE* f, const char* parent, const char* settingName, int value,
                                  const char* srceFilename, int lineNum)
{
  UNUSED(srceFilename);
  UNUSED(lineNum);
  fprintf(f, "%s.%s = %d\n", parent, settingName, value);
}

static inline void saveFloatSetting(FILE* f, const char* parent, const char* settingName,
                                    float value, const char* srceFilename, int lineNum)
{
  UNUSED(srceFilename);
  UNUSED(lineNum);
  fprintf(f, "%s.%s = %f\n", parent, settingName, value);
}

static inline void saveIndexedIntSetting(FILE* f, const char* parent, const char* settingName,
                                         int i, int value, const char* srceFilename, int lineNum)
{
  UNUSED(srceFilename);
  UNUSED(lineNum);
  char indexedSettingName[60];
  sprintf(indexedSettingName, "%s_%d", settingName, i);
  saveIntSetting(f, parent, indexedSettingName, value, srceFilename, lineNum);
}

static inline void saveIndexedFloatSetting(FILE* f, const char* parent, const char* settingName,
                                           int i, float value, const char* srceFilename,
                                           int lineNum)
{
  UNUSED(srceFilename);
  UNUSED(lineNum);
  char indexedSettingName[60];
  sprintf(indexedSettingName, "%s_%d", settingName, i);
  saveFloatSetting(f, parent, indexedSettingName, value, srceFilename, lineNum);
}

#endif
