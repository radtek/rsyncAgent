#pragma once

//签名算法标识
//保留0x00040000~0x8000000FF

#define SGD_SM3_RSA		0x00010001
#define SGD_SHA1_RSA	0x00010002
#define SGD_SHA256_RSA	0x00010004
#define SGD_SM3_SM2		0x00020201
#define SGD_RESERVE		0x00040000

//证书解析项标识
//保留0x00000080~0x0000000FF
#define SGD_CERT_VERSION							0x00000001
#define SGD_CERT_SERIAL								0x00000002
#define SGD_CERT_ISSUER								0x00000005
#define SGD_CERT_VALID_TIME							0x00000006
#define SGD_CERT_SUBJECT							0x00000007
#define SGD_CERT_DER_PUBLIC_KEY						0x00000008
#define SGD_CERT_DER_EXTENSIONS						0x00000009
#define SGD_EXT_AUTHORITYKEYIDENTIFIER_INFO			0x00000011
#define SGD_EXT_SUBJECTKEYIDENTIFIER_INFO			0x00000012
#define SGD_EXT_KEYUSAGE_INFO						0x00000013
#define SGD_EXT_PRIVATEKEYUSAGEPERIOD_INFO			0x00000014
#define SGD_EXT_CERTIFICATEPOLICIES_INFO			0x00000015
#define SGD_EXT_POLICYMAPPINGS_INFO					0x00000016
#define SGD_EXT_BASICCONSTRAINTS_INFO				0x00000017
#define SGD_EXT_POLICYCONSTRAINTS_INFO				0x00000018
#define SGD_EXT_EXTKEYUSAGE_INFO					0x00000019
#define SGD_EXT_CRLDISTRIBUTIONPOINTS_INFO			0x0000001A
#define SGD_EXT_NETSCAPE_CERT_TYPE_INFO				0x0000001B
#define SGD_EXT_SELFDEFINED_EXTENSION_INFO			0x0000001C
#define SGD_CERT_ISSUER_CN							0x00000021
#define SGD_CERT_ISSUER_O							0x00000022
#define SGD_CERT_ISSUER_OU							0x00000023
#define SGD_CERT_SUBJECT_CN							0x00000031
#define SGD_CERT_SUBJECT_O							0x00000032
#define SGD_CERT_SUBJECT_OU							0x00000033
#define SGD_CERT_SUBJECT_EMAIL						0x00000034

//设备信息标签
#define SGD_DEVICE_SORT								0x00000201
#define SGD_DEVICE_TYPE								0x00000202
#define	SGD_DEVICE_NAME								0x00000203
#define SGD_DEVICE_MANUFACTURER						0x00000204
#define SGD_DEVICE_HARDWARE_VERSION					0x00000205
#define SGD_DEVICE_SOFTWARE_VERSION					0x00000206
#define SGD_DEVICE_STANDARD_VERSION					0x00000207
#define SGD_DEVICE_SERIAL_NUMBER					0x00000208
#define SGD_DEVICE_SUPPORT_ALG						0x00000209
#define SGD_DEVICE_SUPPORT_SALG						0x0000020A
#define SGD_DEVICE_SUPPORT_HASH_ALG					0x0000020B
#define SGD_DEVICE_SUPPORT_STORAGE_SPACE			0x0000020C
#define SGD_DEVICE_SUPPORT_FREE_SPACE				0x0000020D
#define SGD_DEVICE_RUNTIME							0x0000020E
#define SGD_DEVICE_USED_TIMES						0x0000020F
#define SGD_DEVICE_LOCATION							0x00000210
#define SGD_DEVICE_DESCRIPTION						0x00000211
#define SGD_DEVICE_MANAGER_INFO						0x00000212
#define SGD_DEVICE_MAX_DATA_SIZE					0x00000213

//对称加密算法
#define SGD_SM1_ECB									0x00000101
#define SGD_SM1_CBC									0x00000102
#define SGD_SM1_CFB									0x00000104
#define SGD_SM1_OFB									0x00000108
#define SGD_SM1_MAC									0x00000110
#define SGD_SSF33_ECB								0x00000201
#define SGD_SSF33_CBC								0x00000202
#define SGD_SSF33_CFB								0x00000204
#define SGD_SSF33_OFB								0x00000208
#define SGD_SSF33_MAC								0x00000210
#define SGD_SM4_ECB									0x00000401
#define SGD_SM4_CBC									0x00000402
#define SGD_SM4_CFB									0x00000404
#define SGD_SM4_OFB									0x00000408
#define SGD_SM4_MAC									0x00000410
#define SGD_ZUC_EEA3								0x00000801
#define SGD_ZUC_EIA3								0x00000802

//证书信息
#define SGD_CERT_VERSION 0x00000001
#define SGD_CERT_SERIAL 0x00000002
#define SGD_CERT_ISSUER 0x00000005
#define SGD_CERT_VALID_TIME 0x00000006
#define SGD_CERT_SUBJECT 0x00000007
#define SGD_CERT_DER_PUBLIC_KEY 0x00000008
#define SGD_CERT_DER_EXTENSIONS 0x00000009
#define SGD_EXT_AUTHORITYKEYIDENTIFIER_INFO 0x00000011
#define SGD_EXT_SUBJECTKEYIDENTIFIER_INFO 0x00000012
#define SGD_EXT_KEYUSAGE_INFO 0x00000013
#define SGD_EXT_PRIVATEKEYUSAGEPERIOD_INFO 0x00000014
#define SGD_EXT_CERTIFICATEPOLICIES_INFO 0x00000015
#define SGD_EXT_POLICYMAPPINGS_INFO 0x00000016
#define SGD_EXT_BASICCONSTRAINTS_INFO 0x00000017
#define SGD_EXT_POLICYCONSTRAINTS_INFO 0x00000018
#define SGD_EXT_EXTKEYUSAGE_INFO 0x00000019
#define SGD_EXT_CRLDISTRIBUTIONPOINTS_INFO 0x0000001A
#define SGD_EXT_NETSCAPE_CERT_TYPE_INFO 0x0000001B
#define SGD_EXT_SELFDEFINED_EXTENSION_INFO 0x0000001C
#define SGD_CERT_ISSUER_CN 0x00000021
#define SGD_CERT_ISSUER_O 0x00000022
#define SGD_CERT_ISSUER_OU 0x00000023
#define SGD_CERT_SUBJECT_CN 0x00000031
#define SGD_CERT_SUBJECT_O 0x00000032
#define SGD_CERT_SUBJECT_OU 0x00000033
#define SGD_CERT_SUBJECT_EMAIL 0x00000034
/// 自定义的OID
#define SGD_OID_IDENTIFY_NUMBER 0x01100034

//设备信息标识
#define SGD_DEVICE_SERIAL_NUMBER	0x00000208

//非对称加解密算法
//保留0x00000080~0x0000000FF
#define SGD_RSA				0x00010000
#define SGD_SM2				0x00020100
#define SGD_SM2_2			0x00020200
#define SGD_SM2_3			0x00020400
#define Reversed			0x00000080~0x0000000FF