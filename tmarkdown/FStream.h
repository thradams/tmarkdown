#pragma once
#include "Result.h"
#include "StringEx.h"
#include <stdio.h>

typedef struct
{
  FILE * pFile;
  StringC fileName;
  int line;
  int col;
} FStream;


void FStream_Destroy(FStream* pfStream);

Result FStream_Init(FStream* pfStream);

Result FStream_InitOpen(FStream* pfStream,
                        const char* fileName);

Result FStream_Open(FStream* pfStream,
                    const char* fileName);

Result FStream_Get(FStream* pfStream, wchar_t* ch);

Result FStream_Unget(FStream* pfStream, wchar_t ch);


typedef struct
{
  const wchar_t* pCharacteres;
  const wchar_t* pCurrentChar;
  wchar_t        putBackCharacter;
  StringC        name;
  int line;
  int col;
} SStream;


void SStream_Destroy(SStream* pfStream);


Result SStream_Init(SStream* pfStream,
                    const char* name,
                    const wchar_t* psz);


Result SStream_Get(SStream* pfStream, wchar_t* ch);

Result SStream_Unget(SStream* pfStream, wchar_t ch);



typedef struct
{
  int type;
  union
  {
    SStream sstream;
    FStream fstream;
  } streamObj;
} Stream;


void Stream_Destroy(Stream* pfStream);


Result Stream_InitOpen(Stream* pfStream,
                    const char* fileName);

Result Stream_InitStr(Stream* pfStream,
                      const char* name,
                       const wchar_t* psz);


const char* Stream_GetName(Stream* pfStream);

Result Stream_Get(Stream* pfStream, wchar_t* ch);

Result Stream_Unget(Stream* pfStream, wchar_t ch);

Result Stream_GetLineCol(Stream* pfStream, int* line, int* col);

FStream* Stream_CastFStream(Stream* pfStream);
