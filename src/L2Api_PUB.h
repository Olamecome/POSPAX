/*****************************************************/
/* L2PublicApi.h                                      */
/* Define the Application Program Interface           */
/* of Public for all PAX Readers                      */
/* Created by sunwei March 6,2019                     */
/*****************************************************/
#ifndef _L2_PUBLIC_API_H
#define _L2_PUBLIC_API_H
int PUB_ReadVerInfo(char *paucVer);
int PUB_MallocDatabase(void);
void PUB_FreeDatabase(void);
#endif

