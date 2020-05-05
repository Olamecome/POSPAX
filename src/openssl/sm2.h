#ifndef _AX_SM2_H_
#define _AX_SM2_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    unsigned char privkey[32];
    unsigned char pubkey[64];
} SM2_KEY;

/*********************************    Gen Za   ************************************
*Input:   entla       ENTLa参数缓冲区地址                       										*
*         entla_len   ENTLa参数长度                             										*
*         ida         IDa参数缓冲区地址                         										*
*         ida_len     IDa参数长度                               										*
*         pubkey      公钥参数缓冲区地址                        										*
*                                                               									*
* 返回值: 0 ---- 表示成功			                            												*
**********************************************************************************/
int Gen_Za(unsigned char *entla,int entla_len,unsigned char *ida,int ida_len,char *pubkey);

/*********************************    Gen SM2 key   *******************************
*Input:   sm2key       sm2 key参数缓冲区地址                    										*
*                                                               									*
*返回值: 0 ---------- 表示成功			                        												*
*        其余值 ----- 表示失败                                  										*
**********************************************************************************/
int Gen_SM2_key(SM2_KEY *sm2key);

/*******************************    SM2 Encrypt   *********************************
*Input:   privkey       私钥                                    										*
*         input         待签名的数据                            										*
*         input_len     待签名的数据长度                        										*
*Output:  sig           加密后的数据                            										*
*		  sig_len       加密后数据长度                          												*
*返回值:  0-------------表示成功			                    													*
*         其余值 ------ 表示失败                               										*
**********************************************************************************/
int SM2_Sign(unsigned char *privkey,unsigned char *input,int input_len,unsigned char *sig, unsigned int *sig_len);

/*******************************    SM2 Decrypt   *********************************
*Input:   pubkey        公钥                                    										*
*         input         待校验的消息                            										*
*         input_len     待校验的消息长度                        										*
*         sig           待校验的消息的签名                      										*
*		  sig_len       待校验的消息的签名的长度                												*
*返回值:  0-------------表示成功			                    													*
*         其余值 ------ 表示失败                                										*
**********************************************************************************/
int SM2_Verify(unsigned char *pubkey,unsigned char *input,int input_len,const unsigned char *sig, int siglen);

/*******************************    SM2 Pubkey encrypt   **************************
*Input:   privkey       公钥                                   										*
*         input         待加密的数据缓冲区地址                  										*
*         input_len     待加密的数据长度                        										*
*         output        输出数据缓冲区地址                      										*
*		  output_len    输出数据的长度                          												*
*返回值:  0-------------表示成功			                    													*
*         其余值 ------ 表示失败                                										*
**********************************************************************************/
int SM2_PubKey_Encrypt(unsigned char *pubkey,unsigned char *input,int input_len, unsigned char *output, int *output_len);

/*******************************    SM2 Privkey Decrypt   *************************
*Input:   privkey       私钥                                   										*
*         input         待解密的数据缓冲区地址                  										*
*         input_len     待解密的数据长度                        										*
*         output        输出数据缓冲区地址                      										*
*		  output_len    输出数据的长度                          												*
*返回值:  0-------------表示成功			                    													*
*         其余值 ------ 表示失败                                										*
**********************************************************************************/
int SM2_PrivKey_Decrypt(unsigned char *privkey,unsigned char *intput, int input_len,unsigned char *output,int *output_len);

#ifdef __cplusplus
}
#endif

#endif
