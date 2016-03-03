

#include "stdafx.h"
#include "FStream.h"
#include "WStrBuilder.h"
#include "CppLex.h"
#include "system.h"
#include "MarkupLex.h"
#include <string.h>
#include <assert.h>

typedef struct
{
  bool bInsideP;
  bool bInsideList;
} State;

void CloseP_Or_List(FILE* pFile, State* state)
{
  if (state->bInsideP)
  {
    state->bInsideP = false;
    fwprintf(pFile, L"</p>\n");
  }
  if (state->bInsideList)
  {
    state->bInsideList = false;
    fwprintf(pFile, L"</ul>\n");
  }
}

void PrintHtmlN(FILE* pFile, const wchar_t* psz, int n)
{
  int k = 0;
  while (*psz)
  {
    switch (*psz)
    {
      case L'&':
        fwprintf(pFile, L"&amp;");
        break;
      case L'<':
        fwprintf(pFile, L"&lt;");
        break;
      case L'>':
        fwprintf(pFile, L"&gt;");
        break;
      case L'"':
        fwprintf(pFile, L"&quot;");
        break;
      default:
        fputwc(*psz, pFile);
    }
    psz++;
    k++;
    if (k == n)
      break;
  }
}

void PrintHtml(FILE* pFile, const wchar_t* psz)
{
  while (*psz)
  {
    switch (*psz)
    {
      case L'&':
        fwprintf(pFile, L"&amp;");
        break;
      case L'<':
        fwprintf(pFile, L"&lt;");
        break;
      case L'>':
        fwprintf(pFile, L"&gt;");
        break;
      case L'"':
        fwprintf(pFile, L"&quot;");
        break;
      default:
        fputwc(*psz, pFile);
    }
    psz++;
  }
}


static Result NextTokenCore(Stream* stream,
                            WStrBuilder* strBuilder,
                            Cpp_Tokens* tk)
{
  int lastGoodState = -2;
  int currentState = 0;
  wchar_t ch;
  //  int index = 0;
  Result r = Stream_Get(stream, &ch);
  while (r == RESULT_OK)
  {
    currentState = Cpp_GetNext(currentState, ch);
    if (lastGoodState == -2 &&
        currentState == -1)
    {
      return RESULT_FAIL;
    }
    if (currentState == -1)
    {
      Stream_Unget(stream, ch);
      break;
    }
    Cpp_Tokens tk2;
    if (Cpp_GetTokenFromState(currentState, &tk2))
    {
      *tk = tk2;
      lastGoodState = currentState;
    }
    WStrBuilder_AppendChar(strBuilder, ch);
    r = Stream_Get(stream, &ch);
  }
  if (r == RESULT_EOF)
  {
    if (lastGoodState != -2)
    {
      r = RESULT_OK;
    }
  }
  return r;
}


static Result Markup_NextTokenCore(Stream* stream,
                                   WStrBuilder* strBuilder,
                                   Markup_Tokens* tk)
{
  int lastGoodState = -2;
  int currentState = 0;
  wchar_t ch;
  //  int index = 0;
  Result r = Stream_Get(stream, &ch);
  while (r == RESULT_OK)
  {
    currentState = Markup_GetNext(currentState, ch);
    if (lastGoodState == -2 &&
        currentState == -1)
    {
      return RESULT_FAIL;
    }
    if (currentState == -1)
    {
      Stream_Unget(stream, ch);
      break;
    }
    Markup_Tokens tk2;
    if (Markup_GetTokenFromState(currentState, &tk2))
    {
      *tk = tk2;
      lastGoodState = currentState;
    }
    WStrBuilder_AppendChar(strBuilder, ch);
    r = Stream_Get(stream, &ch);
  }
  if (r == RESULT_EOF)
  {
    if (lastGoodState != -2)
    {
      r = RESULT_OK;
    }
  }
  return r;
}

Result ReadLine(Stream* stream, WStrBuilder* strBuilder)
{
  WStrBuilder_Clear(strBuilder);

  Result result = RESULT_OK;
  while (result == RESULT_OK)
  {
    wchar_t ch;
    result = Stream_Get(stream, &ch);
    if (result == RESULT_OK)
    {
      if (ch == L'\n')
        break;

      if (ch == L'\0')
      {
        result = RESULT_EOF;
        break;
      }


      result = WStrBuilder_AppendChar(strBuilder, ch);
    }
  }

  return result;
}

bool ProcessPCore(WStrBuilder* strBuilder,
                  FILE* pFile,
                  size_t n)
{
  Stream ss;
  Result result = Stream_InitStr(&ss, "name", strBuilder->c_str + n);
  if (result == RESULT_OK)
  {
    WStrBuilder sb;
    result = WStrBuilder_Init(&sb, 200);
    if (result == RESULT_OK)
    {
      Markup_Tokens tk;
      bool bBold = false;
      bool bItalic = false;
      for (;;)
      {
        WStrBuilder_Clear(&sb);
        result = Markup_NextTokenCore(&ss,
                                      &sb,
                                      &tk);
        if (result != RESULT_OK)
        {
          break;
        }
        switch (tk)
        {
          case TKBold:
            if (bBold)
            {
              fwprintf(pFile, L"</strong>");
              bBold = false;
            }
            else
            {
              fwprintf(pFile, L"<strong>");
              bBold = true;
            }
            break;
          case TKItalic:
            if (bItalic)
            {
              fwprintf(pFile, L"</i>");
              bItalic = false;
            }
            else
            {
              fwprintf(pFile, L"<i>");
              bItalic = true;
            }
            break;
          case TKImage:
          {
            wchar_t* p = wcsstr(sb.c_str, L"](");
            if (p != NULL)
            {
              fwprintf(pFile, L"<img src=\"");
              PrintHtmlN(pFile, p + 2, (sb.c_str + sb.size) - p - 1 - 2); //-2 md
              //PrintHtml(pFile, L"html");
              fwprintf(pFile, L"\">");
              //PrintHtmlN(pFile, sb.c_str + 1, p - sb.c_str + 1 - 1 - 1);
              //fwprintf(pFile, L"</a>");
            }
          }
          break;

          case TKLink:
            fwprintf(pFile, L"<a href=\"");
            PrintHtml(pFile, sb.c_str);
            fwprintf(pFile, L"\">");
            PrintHtml(pFile, sb.c_str);
            fwprintf(pFile, L"</a>");
            break;
          case TKLink2:
          {
            wchar_t* p = wcsstr(sb.c_str, L"](");
            if (p != NULL)
            {
              fwprintf(pFile, L"<a href=\"");
              PrintHtmlN(pFile, p + 2, (sb.c_str + sb.size) - p - 1 - 2 - 2); //-2 md
              PrintHtml(pFile, L"htm");
              fwprintf(pFile, L"\">");
              PrintHtmlN(pFile, sb.c_str + 1, p - sb.c_str + 1 - 1 - 1);
              fwprintf(pFile, L"</a>");
            }
          }
          break;
          default:
            PrintHtml(pFile, sb.c_str);
            break;
        }
      }
      WStrBuilder_Destroy(&sb);
    }

    Stream_Destroy(&ss);
  }



  //TODO
  //fazer parse dos links, bold, italic, imagem

  return true;
}


bool ProcessP(WStrBuilder* strBuilder,
              FILE* pFile,
              State* state)
{
  if (!state->bInsideP)
  {
    state->bInsideP = true;
    fwprintf(pFile, L"<p>");
  }

  return ProcessPCore(strBuilder,
                    pFile,
                    0);
}

bool IsKeywordCpp(const wchar_t* psz)
{
  const wchar_t* kws[] =
  {
    L"alignas",
    L"alignof",
    L"and",
    L"and_eq",
    L"asm",
    L"auto",
    L"bitand",
    L"bitor",
    L"bool",
    L"break",
    L"case",
    L"catch",
    L"char",
    L"char16_t",
    L"char32_t",
    L"class",
    L"compl",
    L"concept",
    L"const",
    L"constexpr",
    L"const_cast",
    L"continue",
    L"decltype",
    L"default",
    L"delete",
    L"do",
    L"double",
    L"dynamic_cast",
    L"else",
    L"enum",
    L"explicit",
    L"export",
    L"extern",
    L"false",
    L"float",
    L"for",
    L"friend",
    L"goto",
    L"if",
    L"inline",
    L"int",
    L"long",
    L"mutable",
    L"namespace",
    L"new",
    L"noexcept",
    L"not",
    L"not_eq",
    L"nullptr",
    L"operator",
    L"or",
    L"or_eq",
    L"private",
    L"protected",
    L"public",
    L"register",
    L"reinterpret_cast",
    L"requires",
    L"return",
    L"short",
    L"signed",
    L"sizeof",
    L"static",
    L"static_assert",
    L"static_cast",
    L"struct",
    L"switch",
    L"template",
    L"this",
    L"thread_local",
    L"throw",
    L"true",
    L"try",
    L"typedef",
    L"typeid",
    L"typename",
    L"union",
    L"unsigned",
    L"using",
    L"virtual",
    L"void",
    L"volatile",
    L"wchar_t",
    L"while",
    L"xor",
    L"xor_eq",
    L"final",
    L"override",
    L"_Pragma",
    L"#include",
    L"#ifdef",
    L"#pragma",
  };
  //http://en.cppreference.com/w/c/keyword
  for (size_t i = 0; i < sizeof(kws) / sizeof(kws[0]); i++)
  {
    if (wcscmp(kws[i], psz) == 0)
      return true;
  }
  return false;
}


bool ProcessLine(WStrBuilder* strBuilder,
                 FILE* pFile,
                 State* state)
{
 

  if (wcsncmp(strBuilder->c_str, L"---", 3) == 0)
  { 
    CloseP_Or_List(pFile, state);
    fwprintf(pFile, L"<hr>\n");
    return true;
  }
  return false;
}

bool ProcessCode(Stream* stream,
                 WStrBuilder* strBuilder,
                 FILE* pFile,
                 State* state,
                 const wchar_t* pszLang,
                 bool (*pfIsKeyword)(const wchar_t* psz))
{
  if (wcsncmp(strBuilder->c_str, pszLang, wcslen(pszLang)) == 0)
  {
    CloseP_Or_List(pFile, state);

    
    fwprintf(pFile, L"<pre>\n");

    Result result = RESULT_OK;

    Cpp_Tokens tk;

    while (result == RESULT_OK)
    {
      WStrBuilder_Clear(strBuilder);
      result = NextTokenCore(stream,
                             strBuilder,
                             &tk);

      if (result == RESULT_OK)
      {
        switch (tk)
        {
          case TKLINECOMMENT:
          case TKCOMMENT:
            fwprintf(pFile, L"<span class=\"comment\">");
            PrintHtml(pFile, strBuilder->c_str);
            fwprintf(pFile, L"</span>");
            break;
          case TKCHAR:
          case TKSTRING:
            fwprintf(pFile, L"<span class=\"string\">");
            PrintHtml(pFile, strBuilder->c_str);
            fwprintf(pFile, L"</span>");
            break;
          case TKIdentifier:

            if (pfIsKeyword(strBuilder->c_str))
            {
              fwprintf(pFile, L"<span class=\"keyword\">");
              PrintHtml(pFile, strBuilder->c_str);
              fwprintf(pFile, L"</span>");
            }
            else
            {
              PrintHtml(pFile, strBuilder->c_str);
            }
            break;
          
          case TKTKNOWIKI_BLOCK_CLOSE:
            break;
          default:
            PrintHtml(pFile, strBuilder->c_str);
            break;

        }
        if (tk == TKTKNOWIKI_BLOCK_CLOSE)
          break;
      }
      else
        break;
    }
    fwprintf(pFile, L"</pre>\n");

    return true;
  }
  return false;
}

bool ProcessHeader(WStrBuilder* strBuilder,
                   FILE* pFile,
                   State* state)
{
  bool bHeader = false;
  //tentar todos os headers
  for (int i = 6; i > 0; i--)
  {
    if (wcsncmp(strBuilder->c_str, L"######", i) == 0)
    {
      CloseP_Or_List(pFile, state);

      fwprintf(pFile, L"<h%d>", i);
      if ((int)strBuilder->size > i)
      {
        fwprintf(pFile, L"%s", strBuilder->c_str + i);
      }
      fwprintf(pFile, L"</h%d>\n", i);
      bHeader = true;
      break;
    }
  }
  return bHeader;
}

bool ProcessPre(Stream* stream,
                WStrBuilder* strBuilder,
                FILE* pFile,
                State* state)
{
  
  if (wcsncmp(strBuilder->c_str, L"```", 3) == 0)
  {
    CloseP_Or_List(pFile, state);

    fwprintf(pFile, L"<pre>\n");
    while (ReadLine(stream, strBuilder) == RESULT_OK)
    {
      if (wcsncmp(strBuilder->c_str, L"```", 3) == 0)
      {
        break;
      }
      PrintHtml(pFile, strBuilder->c_str);
      fwprintf(pFile, L"\n");
    }
    fwprintf(pFile, L"</pre>\n");
    return true;
  }
  return false;
}



void ReadStream(Stream* stream,
                WStrBuilder* strBuilder,
                FILE* pFile)
{
  State state = {false, false};
  

  while (ReadLine(stream, strBuilder) == RESULT_OK)
  {

    if (ProcessHeader(strBuilder,
                      pFile,
                      &state))
    {
      continue;
    }

    if (ProcessCode(stream,
                    strBuilder,
                    pFile,
                    &state,
                    L"```cpp",
                    &IsKeywordCpp))
    {
      continue;
    }

    if (ProcessPre(stream,
                   strBuilder,
                   pFile,
                   &state))
    {
      continue;
    }


    if (ProcessLine(strBuilder,
                    pFile,
                    &state))
    {
      continue;
    }

    if (strBuilder->size > 1 &&
        strBuilder->c_str[0] == L'*' &&
        strBuilder->c_str[1] != L'*')
    {
      if (state.bInsideP)
      {
        state.bInsideP = false;
        fwprintf(pFile, L"</p>\n");
      }

      if (!state.bInsideList)
      {
        state.bInsideList = true;
        fwprintf(pFile, L"<ul>\n");
      }
      fwprintf(pFile, L"<li>");

      ProcessPCore(strBuilder,
               pFile,
               1);

      //PrintHtml(pFile, strBuilder->c_str + 1);
      fwprintf(pFile, L"</li>\n");
    }
    else if (strBuilder->size == 0)
    {
      //process empty line
      CloseP_Or_List(pFile, &state);
      fwprintf(pFile, L"\n");
    }
    else
    {
      if (state.bInsideList)
      {
        state.bInsideList = false;
        fwprintf(pFile, L"</ul>\n");
      }

      ProcessP(strBuilder,
               pFile,
               &state);
    }

  }
}

Result AppendFile(FILE* pFile, const char* fileName)
{
  if (fileName == NULL)
  {
    return RESULT_OK;
  }

  Result result = RESULT_OK;
  FILE* pFileIn = fopen(fileName, "r,ccs=UTF-8");
  if (pFile != NULL)
  {
    for (;;)
    {
      wint_t c = fgetwc(pFileIn);

      if (ferror(pFileIn) != 0)
      {
        result = RESULT_FAIL;
        break;
      }

      if (c == WEOF)
      {
        result = RESULT_EOF;
        break;
      }

      fputwc(c, pFile);
    }

    fclose(pFileIn);
  }
  return result;
}

void ProcessFile(const char* inputFileName,
                 const char* outputFileName,
                 const char* headerFile,
                 const char* footerFile)
{
  char outFileName[200];

  if (outputFileName == NULL)
  {
    strncpy(outFileName, inputFileName, 199);
    outFileName[strlen(outFileName) - 2] = 0;
    /*2 = sizeof(md)*/
    strcat(outFileName, "htm");
    outputFileName = outFileName;
  }



  Stream stream;
  Result result = Stream_InitOpen(&stream, inputFileName);
  if (result == RESULT_OK)
  {
    WStrBuilder  strBuilder;
    result = WStrBuilder_Init(&strBuilder, 300);
    if (result == RESULT_OK)
    {
      FILE* pFile = fopen(outputFileName, "w,ccs=UTF-8");
      if (pFile != NULL)
      {
        AppendFile(pFile, headerFile);
        ReadStream(&stream, &strBuilder, pFile);
        AppendFile(pFile, footerFile);

        fclose(pFile);
      }

      WStrBuilder_Destroy(&strBuilder);
    }
    Stream_Destroy(&stream);
  }
}
void PrintUsage()
{

}

const char* g_headerFile = NULL;
const char* g_footerFile = NULL;

void ForThisFile(const char* fileName)
{
  ProcessFile(fileName, NULL, g_headerFile, g_footerFile);
}

int main(int argc, char* argv[])
{
  bool bAll = false;

  for (int i = 0; i < argc; i++)
  {
    if (strcmp(argv[i], "-all") == 0)
    {
      bAll = true;
    }
    else if (strcmp(argv[i], "-h") == 0)
    {
      if (i + 1 >= argc)
      {
        PrintUsage();
        break;
      }
      g_headerFile = argv[i + 1];
      i++;

    }
    else if (strcmp(argv[i], "-f") == 0)
    {
      g_footerFile = argv[i + 1];
      i++;
    }
    else if (strcmp(argv[i], "-o") == 0)
    {
      g_footerFile = argv[i + 1];
      i++;
    }
  }
  //para 1 so


  //para varios
  if (bAll)
  {
    ForEachFile(".\\", &ForThisFile);
  }
  else
  {
    ProcessFile(argv[1], argv[2], g_headerFile, g_footerFile);
  }
  return 0;
}

