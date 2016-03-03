/*
 * Generated by TKLGEN - VersionNov 27 2015
 * Copyright (C) 2013, Thiago Adams (thiago.adams@gmail.com)
 * www.thradams.com
 *
 */
#ifndef MARKUPLEX_H
#define MARKUPLEX_H



typedef enum 
{
    TKLink,
    TKLink2,
    TKImage,
    TKEscape,
    TKBold,
    TKItalic,
    TKOther,
} Markup_Tokens;

const char* Markup_Tokens_ToString(Markup_Tokens e);


int Markup_GetNext(int state, wchar_t ch);
int Markup_GetTokenFromState(int state, Markup_Tokens* tk);
int Markup_IsInterleave(Markup_Tokens tk);

#endif
