//---------------------------------------------------------------------------
// NAME: hex2bin.h
// DESC: This module implements the hex to binary convert
//
// Copyright (c) IT Experts (Pty) Ltd |YEAR|.  All rights reserved.
//
// $Log: hex2bin.h,v $
// Revision 1.1  2003/09/29 06:38:52  ecarpenter
// Initial checkin
//
//---------------------------------------------------------------------------
#ifndef HEX2BIN_H
#define HEX2BIN_H
//------------------------------------------------------------[ Includes ]---

//-------------------------------------------------------------[ Defines ]---

//----------------------------------------------------------[ Prototypes ]---
#ifdef __cplusplus
extern "C" {
#endif

int hex2bin(const char *pcInBuffer, char *pcOutBuffer, int iLen);

#ifdef __cplusplus
}
#endif

//---------------------------------------------------------------------------
//
//                               T H E   E N D
//
//---------------------------------------------------------------------------
#endif
