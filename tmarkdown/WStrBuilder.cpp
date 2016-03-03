
#include "stdafx.h"

#include "WStrBuilder.h"
#include <string.h>
#include <stdlib.h>

#include <stdlib.h>

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stddef.h>
#include "dmalloc.h"
#include "assert.h"

#ifdef USE_UTF8
#include  "Utf8.h"
#endif
#include "StringEx.h"

static int IsEmptyStr(const wchar_t* psz)
{
    return psz == NULL || psz[0] == '\0';
}

Result WStrBuilder_Init(WStrBuilder* p, size_t capacity)
{
    Result result = RESULT_OK;
    p->c_str = NULL;
    p->size = 0;
    p->capacity = 0;

    if (capacity > 0)
    {
        result = WStrBuilder_Reserve(p, capacity);
    }

    return result;
}

void WStrBuilder_Swap(WStrBuilder* str1,
                     WStrBuilder* str2)
{
    WStrBuilder temp;
    memcpy(&temp, str1, sizeof(WStrBuilder));
    memcpy(str1, str2, sizeof(WStrBuilder));
    memcpy(str2, &temp, sizeof(WStrBuilder));
}

void WStrBuilder_Destroy(WStrBuilder* p)
{
    if (p)
    {
        Free(p->c_str);
        p->c_str = NULL;
        p->size = 0;
        p->capacity = 0;
    }
}

Result WStrBuilder_Reserve(WStrBuilder* p, size_t nelements)
{
    Result r = RESULT_OK;

    if (nelements > p->capacity)
    {
        wchar_t* pnew = (wchar_t*)Realloc(p->c_str,
                                    (nelements + 1) * sizeof(p->c_str[0]));

        if (pnew)
        {
            if (p->c_str == NULL)
            {
                pnew[0] = '\0';
            }

            p->c_str = pnew;
            p->capacity = nelements;
        }
        else
        {
            r = RESULT_OUT_OF_MEM;
        }
    }

    return r;
}

static Result Grow(WStrBuilder* p, size_t nelements)
{
    Result r = RESULT_OK;

    if (nelements > p->capacity)
    {
        size_t new_nelements = p->capacity + p->capacity / 2;

        if (new_nelements < nelements)
        {
            new_nelements = nelements;
        }

        r = WStrBuilder_Reserve(p, new_nelements);
    }

    return r;
}

Result WStrBuilder_SetN(WStrBuilder* p,
                       const wchar_t* source,
                       size_t nelements)
{
    Result r = Grow(p, nelements);

    if (r == 0)
    {
      wcsncpy_s(p->c_str, p->capacity + 1, source, nelements);
        p->c_str[nelements] = '\0';
        p->size = nelements;
    }

    return r;
}

Result WStrBuilder_Set(WStrBuilder* p,
                      const wchar_t* source)
{
    WStrBuilder_Clear(p);
    return WStrBuilder_SetN(p, source, wcslen(source));
}

Result WStrBuilder_AppendN(WStrBuilder* p,
                          const wchar_t* source,
                          size_t nelements)
{
    if (IsEmptyStr(source))
    {
        return RESULT_OK;
    }

    Result r = Grow(p, p->size + nelements);

    if (r == RESULT_OK)
    {
      wcsncpy_s(p->c_str + p->size,
                  (p->capacity + 1) - p->size,
                  source,
                  nelements);
        p->c_str[p->size + nelements] = '\0';
        p->size += nelements;
    }

    return r;
}

Result WStrBuilder_Append(WStrBuilder* p,
                         const wchar_t* source)
{
    if (IsEmptyStr(source))
    {
        return RESULT_OK;
    }

    return WStrBuilder_AppendN(p, source, wcslen(source));
}

void WStrBuilder_Clear(WStrBuilder* p)
{
    if (p->c_str != NULL)
    {
        p->c_str[0] = '\0';
        p->size = 0;
    }
}

wchar_t* WStrBuilder_Release(WStrBuilder* p)
{
    wchar_t* pResult = p->c_str;

    if (pResult != NULL)
    {
        p->c_str = NULL;
        p->size = 0;
        p->capacity = 0;
    }

    return pResult;
}

Result WStrBuilder_AppendInt(WStrBuilder* p, int i)
{
    Result r = RESULT_FAIL;
    wchar_t buffer[40];
#ifdef ITOA
    itoa(i, buffer, 10);
#else
    _itow_s(i, buffer, sizeof(buffer), 10);
#endif
    r = WStrBuilder_Append(p, buffer);
    return r;
}

void WStrBuilder_Attach(WStrBuilder* pWStrBuilder,
                       wchar_t* psz,
                       size_t nBytes)
{
    if (psz != NULL)
    {
        ASSERT(nBytes > 0);
        WStrBuilder_Destroy(pWStrBuilder);
        pWStrBuilder->c_str = psz;
        pWStrBuilder->capacity = nBytes - 1;
        pWStrBuilder->size = 0;
        pWStrBuilder->c_str[0] = '\0';
    }
}


Result WStrBuilder_AppendChar(WStrBuilder* p, wchar_t ch)
{
    return WStrBuilder_AppendN(p, &ch, 1);
}


