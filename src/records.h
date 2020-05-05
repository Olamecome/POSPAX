
#ifndef __RECORD_H__
#define __RECORD_H__


int reprintLast();
int reprintAll();
int reprintSequenceNumber();
int reprintSTAN();
int reprintRRN();
int repushTransactions(char silent);


int dailySummary();
int weeklySummary();
int yearlySummary();


int closeBatch();

int processEOD(int option);



#endif // !__RECORD_H__