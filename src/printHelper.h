#pragma once
#ifndef __PRINT_HELPER__
#define __PRINT_HELPER__

#include "global.h"


int printTransactionReceipt(TRAN_LOG* transData, int copy, uchar reprint);
int checkPrinter();
int printTerminalDetails();
int printSummaryReport(SummaryReport* summary, char* title);
int printEODReport(SummaryReport* summaryReport, char* title, int type);


#endif // !__PRINT_HELPER__
