#include "global.h"


extern int transaction(uchar tranType);
extern int servicesMenu();

int route(int selection) {
	switch (selection) {
	case BALANCE:
	case CASH_ADVANCE:
	case PURCHASE:
	case PURCHASE_CASH:
	case PURCHASE_WITH_CASH_BACK:
	case PURCHASE_WITH_ADDITIONAL_DATA:
	case POS_PRE_AUTHORIZATION:
	case POS_PRE_AUTH_COMPLETION:
	case REFUND:
	case REVERSAL:
	case VOID:
		return transaction(selection);
		break;
	case REPORTING:
		return servicesMenu();
		break;
	}

	return 0;
}