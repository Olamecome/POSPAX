
/****************************************************************************
NAME
    password.h -

DESCRIPTION

REFERENCE

MODIFICATION SHEET:
    MODIFIED   (YYYY.MM.DD)
    shengjx     2006.09.12      - created
****************************************************************************/

#ifndef _PASSWORD_H
#define _PASSWORD_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void ResetPwdAll(void);
uchar PasswordBank(void);
uchar PasswordTerm(void);
uchar PasswordMerchant(void);
uchar PasswordVoid(void);
uchar PasswordRefund(void);
uchar PasswordAdjust(void);
uchar PasswordSettle(void);

int ModifyPasswordBank(void);
int ModifyPasswordTerm(void);
int ModifyPasswordMerchant(void);
int ModifyPasswordVoid(void);
int ModifyPasswordRefund(void);
int ModifyPasswordAdjust(void);
int ModifyPasswordSettle(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// _PASSWORD_H

// end of file
