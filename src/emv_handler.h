#pragma once


void resetTransactionData();

int startEmvTransaction(unsigned short uiEntryMode, int ucTranType, char amount[12 + 1], char otherAmount[12 + 1]);