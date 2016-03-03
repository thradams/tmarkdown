

#pragma once

#include <string.h>
#include "Result.h"

typedef struct
{
    wchar_t*  c_str;
    size_t size;
    size_t capacity;

} WStrBuilder;

#define STRBUILDER_INIT { 0, 0, 0 }
#define STRBUILDER_DEFAULT_SIZE 17

Result WStrBuilder_Init(WStrBuilder* p, size_t capacity);

Result WStrBuilder_Reserve(WStrBuilder* p, size_t nelements);

void WStrBuilder_Attach(WStrBuilder* wstr, 
                       wchar_t* psz,
                       size_t nBytes);

void WStrBuilder_Destroy(WStrBuilder* wstr);
void WStrBuilder_Swap(WStrBuilder* str1, WStrBuilder* str2);


void WStrBuilder_Clear(WStrBuilder* wstr);

Result WStrBuilder_SetN(WStrBuilder* p,
                       const wchar_t* source,
                       size_t nelements);

Result WStrBuilder_Set(WStrBuilder* p,
                      const wchar_t* source);

Result WStrBuilder_AppendN(WStrBuilder* p,
                          const wchar_t* source,
                          size_t nelements);

Result WStrBuilder_Append(WStrBuilder* p,
                         const wchar_t* source);

wchar_t* WStrBuilder_Release(WStrBuilder* p);

Result WStrBuilder_AppendInt(WStrBuilder * p, wchar_t i);

Result WStrBuilder_AppendChar(WStrBuilder * p, wchar_t wch);

