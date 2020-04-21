#include "goom_testing.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static void read_line(char** line, FILE* f)
{
  size_t len = 0;
  if (getline(line, &len, f) == -1) {
    printf("Could not read ifs settings line.");
    exit(1);
  }
}

static void getFullSettingNameAndFormat(const char* parent, const char* settingName,
                                        const char* formatSpecifier, char* fullSettingName,
                                        char* settingFormat)
{
  strcpy(fullSettingName, parent);
  strcat(fullSettingName, ".");
  strcat(fullSettingName, settingName);
  strcpy(settingFormat, fullSettingName);
  strcat(settingFormat, " = ");
  strcat(settingFormat, formatSpecifier);
  strcat(settingFormat, "\n");
}

static char fullSettingName[50];
static char settingFormat[100];

int getIntSetting(FILE* f, const char* parent, const char* settingName, const char* srceFilename,
                  int lineNum)
{
  char* line = NULL;
  read_line(&line, f);

  getFullSettingNameAndFormat(parent, settingName, "%d", fullSettingName, settingFormat);

  int value;
  const int matched = sscanf(line, settingFormat, &value);
  if (matched != 1) {
    printf("File %s, line %d: sscanf = %d; Could not read setting \"%s\" from line \"%s\" with fmt "
           "= \"%s\".",
           srceFilename, lineNum, matched, fullSettingName, line, settingFormat);
    free(line);
    exit(1);
  }

  free(line);
  return value;
}

float getFloatSetting(FILE* f, const char* parent, const char* settingName,
                      const char* srceFilename, int lineNum)
{
  char* line = NULL;
  read_line(&line, f);

  getFullSettingNameAndFormat(parent, settingName, "%f", fullSettingName, settingFormat);

  float value;
  const int matched = sscanf(line, settingFormat, &value);
  if (matched != 1) {
    printf("File %s, line %d: sscanf = %d; Could not read setting \"%s\" from line \"%s\" with fmt "
           "= \"%s\".",
           srceFilename, lineNum, matched, fullSettingName, line, settingFormat);
    free(line);
    exit(1);
  }
  free(line);
  return value;
}

int getIndexedIntSetting(FILE* f, const char* parent, const char* settingName, int i,
                         const char* srceFilename, int lineNum)
{
  char indexedSettingName[60];
  sprintf(indexedSettingName, "%s_%d", settingName, i);
  return getIntSetting(f, parent, indexedSettingName, srceFilename, lineNum);
}

float getIndexedFloatSetting(FILE* f, const char* parent, const char* settingName, int i,
                             const char* srceFilename, int lineNum)
{
  char indexedSettingName[60];
  sprintf(indexedSettingName, "%s_%d", settingName, i);
  return getFloatSetting(f, parent, indexedSettingName, srceFilename, lineNum);
}
