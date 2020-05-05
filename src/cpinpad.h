#ifndef _PIN_PAD_H
#define _PIN_PAD_H

#define PP_OPEN         0x01       
#define PP_CLOSE        0x00
#define PP_GREEN        0x01
#define PP_RED          0x02
#define PP_YELLOW	    0x04

uchar PPBaudRate(long Baudrate);

// Set Pininput Time 
// precision is 0.01 second
uchar PPSetTimeout(unsigned short timeout);

// beep
uchar PPBeep(void);

/*
  OperMode=O_OPEN  turn on PINPAD light
  OperMode=O_CLOSE turn off PINPAD light
  Index          combination as below
       RED       red light
       GREEN     green light
       YELLOW    yellow light
*/
uchar PPLight(uchar OperMode, uchar Index);

/*
  OperMode=O_OPEN  turn on PINPAD voice prompt
  OperMode=O_CLOSE turn off PINPAD voice prompt
*/
uchar PPInvoice(uchar OperMode);

/*
  OperMode=O_OPEN  turn on PINPAD key tone
  OperMode=O_CLOSE turn off PINPAD key tone
*/
uchar PPKbmute(uchar OperMode);


/*
  OperMode=O_OPEN  turn on back light
  OperMode=O_CLOSE turn off back light
*/
uchar PPBackLight(uchar OperMode);

//Input for reserve
/*
  function  for entering string
  str       string
  min       min string length
  max       max string length
  mode=0    clear text
  mode=1    replace string displayed with '*'
*/
uchar PPInput(uchar *strInOut, uchar min, uchar max, uchar mode);

// LCD Ctrl
// Clear screen
uchar PPScrCls(void);


/*
  function	clear line
  line      line No.(0 - 1)
*/
uchar PPScrClrLine(uchar line);


/*
  function	display string
  line      line No.(0 - 1)
  col       Row Position(0 - 113)
  str       string
*/
uchar PPScrPrint(uchar line, uchar col, const uchar *str);

/*
  function	display dot matrix
  line      line No.(0 - 1)
  col       Row position(0 - 121)
  Data      data(format please check protocol)
  len       data length
*/
uchar PPScrWrData(uchar line, uchar col, const uchar *Data, uchar len);


/*key management
  index of all keys: 1 - 100
*/
/*
  function	write a master key to PINPAD
  KeyID     key index(space of master key)
  Key       key value
  mode=0x01 DES key
  mode=0x03 3DES or SEED key (16bytes)
  mode=0x07	3DES or SEED key (24bytes)
*/
uchar PPWriteMKey(uchar KeyID, uchar mode, const uchar *Key);


/*
  function	write a DES(3DES) key to PINPAD
  KeyID     key index(space of DES key)
  Key       key value
  mode=0x01 DES key
  mode=0x03 3DES or SEED key (16bytes)
  mode=0x07	3DES or SEED key (24bytes)
*/
uchar PPWriteDKey(uchar DESKeyID, uchar mode, const uchar *Key);


/*
  function	write a work key to PINPAD
  MKeyID    the index of master key used to calculate work key, MKeyID=0,write directly without calculation
  WKeyID    key index(space of work key)
  Key       key
  (
  Mode=0x01  DES encryption
  Mode=0x81  DES decryption 
  MKey and WKey are both DES keys.
  )
  (
  Mode=0x03  3DES encryption
  Mode=0x83  3DES decryption
  MKey is 3DES(16 bytes), WKey is 8 bytes.
  )
	
  (
  Mode=0x07  3DES encryption
  Mode=0x87  3DES decryption
  MKey is 3DES(24 bytes), WKey is 8 bytes.
  )
	
  (
  Mode=0x31  3DES encryption
  Mode=0xb1  3DES decryption
  MKey is 3DES(16 bytes), WKey is 8 bytes.
  )

  (
  Mode=0x33  3DES encryption
  Mode=0xb3  3DES decryption
  MKey and WKey are both 3DES(16 bytes), and using EBC mode
  该模式下，MKeyID与WKeyID均为3DES－16字节密钥，并采用EBC三重加解密方式。
  )
	
  (
  Mode=0x71  3DES encryption
  Mode=0xf1  3DES decryption
  MKey is 3DES(24 bytes), WKey is 8 bytes.
  )
	
  (
  Mode=0x73  3DES encryption
  Mode=0xf3  3DES decryption
  MKey is 3DES(24 bytes), WKey is 3DES(16 bytes), using EBC mode
  该模式下，MKeyID为3DES－24字节密钥, WKeyID为3DES－16字节密钥，并采用EBC三重加解密方式。
  )
  
  (
  Mode=0x77  3DES encryption
  Mode=0xf7  3DES decryption
  MKey and WKey are both 3DES(24 bytes), and using EBC mode
  该模式下，MKeyID与WKeyID均为3DES－24字节密钥，并采用EBC三重加解密方式。
  )

*/
uchar PPWriteWKey(uchar MKeyID, uchar WKeyID, uchar mode, const uchar *Key);


/*
  function	Derive key
  KeyID     key needs to be derived high bit is 1 means index of work key
			otherwise means index of master key
  WKeyID1   Distributed data(space of work key)
  WKeyID2   Distributed result(space of work key)
	
	(
	Mode=0x01  DES encryption
	Mode=0x81  DES decryption
	MKey, WKey ID1# and WKey ID2# are all DES keys.
	)

	(
	Mode=0x03  3DES encryption
	Mode=0x83  3DES decryption
	MKey is 3DES(16 bytes), WKey ID1# and WKey ID2# are 8 bytes.
	)

	(
	Mode=0x07  3DES encryption
	Mode=0x87  3DES decryption
	MKey is 3DES(24 bytes), WKey ID1# and WKey ID2# are 8 bytes.
	)

	(
	Mode=0x31  3DES encryption
	Mode=0xb1  3DES decryption
	MKey is 3DES(16 bytes), WKey ID1# and WKey ID2# are 8 bytes.
	)

	(
	Mode=0x33  3DES encryption
	Mode=0xb3  3DES decryption
	MKey, WKey ID1# and WKey ID2# are all 3DES(16 bytes), and using EBC mode
	该模式下，MKeyID与WKeyID1、WKeyID2均为3DES－16字节密钥，并采用EBC三重加解密方式。
	)

	(
	Mode=0x71  3DES encryption
	Mode=0xf1  3DES decryption
	MKey is 3DES(24 bytes), WKey ID1# and WKey ID2# are 8 bytes.
	)

	(
	Mode=0x73  3DES encryption
	Mode=0xf3  3DES decryption
	MKey is 3DES(24 bytes), WKey ID1# and WKey ID2# are 16 bytes, and using EBC mode
	该模式下，MKeyID为3DES－24字节密钥, WKeyID1、WKeyID2为3DES－16字节密钥，并采用EBC三重加解密方式。
	)

	(
	Mode=0x77  3DES encryption
	Mode=0xf7  3DES decryption
	MKey, WKey ID1# and WKey ID2# are all 3DES(24 bytes), and using EBC mode
	该模式下，MKeyID与WKeyID1、WKeyID2均为3DES－24字节密钥，并采用EBC三重加解密方式。
	)
*/
uchar PPDeriveKey(uchar MKeyID, uchar WKeyID1, uchar WKeyID2, uchar mode);


/*
  function	get password encrypted by ANSI9.8
  TwiceInput=1 play voice to prompt entering password twice on PINPAD
  TwiceInput=0 play voice to prompt entering password once on PINPAD
  PinKeyID     index of work key(space of work key)
  min		   min password length
  max          max password length
  pin          encrypted password
  cardno       a 16 digits shifted card no(ASCII)
*/
uchar PPGetPwd(uchar PinKeyID, uchar min, uchar max, uchar *cardno, uchar *pin,uchar TwiceInput);

/*
  function	get password encrypted by ANSI9.8
  TwiceInput=1 play voice to prompt entering password twice on PINPAD
  TwiceInput=0 play voice to prompt entering password once on PINPAD
  PinKeyID     index of work key(space of work key)
  min		   min password length
  max          max password length
  pin          encrypted password
  cardno       a 16 digits shifted card no(ASCII)
  mode:
  mode=0x01		DES encryption
  mode=0x03		3DES encryption(16bytes Des)
  mode=0x07 	3DES encryption(24bytes Des)	
  
	
*/
uchar Ex_PPGetPwd(uchar PinKeyID, uchar min, uchar max, uchar *cardno, uchar *pin,uchar TwiceInput,uchar mode);


/*
  function	DES(3DES) calculation
  DESKeyID  index of DES key(space of DES key)
  datain    input data
  macout    output data
	mode=0x01  DES encryption   [8Bytes key]
	mode=0x81  DES decryption   [8Bytes key]
	mode=0x03  3DES encryption  [16Bytes key]
	mode=0x83  3DES decryption  [16Bytes key]
	mode=0x07  3DES encryption  [24Bytes key]
	mode=0x87  3DES decryption  [24Bytes key]

*/
uchar PPDes(uchar DESKeyID, uchar mode, const uchar *datain, uchar *dataout);
/*
  function	calculate MAC
  flag=0    algorithm 1
  flag=1    algorithm 2
  WKeyID    index of work key(space of work key)
  inLen     data length
  datain    data
  macout    output(MAC)
  mode=0x01 DES encryption
  mode=0x03 3DES encryption(16bytes Des)
  mode=0x07 3DES encryption(24bytes Des)
*/
uchar PPMac(uchar WKeyID,uchar mode, const uchar *datain, ushort inLen, uchar *macout,uchar flag);

uchar PPVerInfo(uchar *ver);

uchar PPUpdLogo(uchar line, const uchar *Data);


/*
函数功能		得到IC卡的明文密码输入，Pos,PP20传输的过程中采用随机数加密
	min          密码可输入最小长度
  max          密码可输入最大长度
  pin_block    明文密码输出
*/
uchar PPEmvGetPwd(uchar min, uchar max,uchar *pin_block);


/*
  函数功能     读取ANSI9.8加密的密码(16字节3des加密的PIN)
  TwiceInput=1 PINPAD语音提示输入2次
  TwiceInput=0 PINPAD语音提示输入1次
  PinKeyID     计算PIN的工作密钥(工作密钥区)
  min          密码可输入最小长度
  max          密码可输入最大长度
  pin          加密的密码输出
  cardno       经过移位处理的16位卡号（ASCII码）
  mode:
  mode=0x31		16字节3des加密
   
*/
uchar PPGetPwd_3Des(uchar PinKeyID, uchar mode,uchar min, uchar max, uchar *cardno, uchar *pin, uchar TwiceInput);


uchar PPQuickBeepTest(void);
uchar PPCancel(void);

#endif
