#include "stdafx.h"
#include "system.h"
#include <Windows.h>
#include "Strsafe.h"

int ForEachFile(const char* directory,
                void (*pCallback)(const char*))
{
  WIN32_FIND_DATAA ffd;
  LARGE_INTEGER filesize;
  char szDir[MAX_PATH];
  size_t length_of_arg;
  HANDLE hFind = INVALID_HANDLE_VALUE;
  DWORD dwError = 0;


  StringCchLengthA(directory, MAX_PATH, &length_of_arg);

  if (length_of_arg > (MAX_PATH - 3))
  {
    //assert(false);
    return (-1);
  }

  StringCchCopyA(szDir, MAX_PATH, directory);
  StringCchCatA(szDir, MAX_PATH, "\\*.md");

  hFind = FindFirstFileA(szDir, &ffd);

  if (INVALID_HANDLE_VALUE == hFind)
  {
    //assert(false);
    return dwError;
  }

  do
  {
    if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
      //Não é recursiva por enquanto ffd.cFileName
    }
    else
    {
      filesize.LowPart = ffd.nFileSizeLow;
      filesize.HighPart = ffd.nFileSizeHigh;

      pCallback(ffd.cFileName);
    }
  } while (FindNextFileA(hFind, &ffd) != 0);

  dwError = GetLastError();
  if (dwError != ERROR_NO_MORE_FILES)
  {
    //assert(false);
    return dwError;
  }

  FindClose(hFind);
  return 0;
}
