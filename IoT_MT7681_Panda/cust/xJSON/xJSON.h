#ifndef XJSON_H
#define XJSON_H

#define XJSON_MAX_STRING_LEN		512

typedef struct xJSON {
	int length;
	char *string;				/* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
} xJSON;

extern void xJSON_InitObject(xJSON * PJSON,char * buff);
extern void xJSON_putInt(xJSON * json,char * key,int value);
extern void xJSON_putString(xJSON * json,char * key,char * value);
extern void xJSON_ObjectStart(xJSON * json,char * key,UCHAR conn);
extern void xJSON_ObjectComplete(xJSON * json);
extern void xJSON_ArrayStart(xJSON * json,char * key,UCHAR conn);
extern void xJSON_ArrayComplete(xJSON * json);

extern void xJSON_putProductType(xJSON * msg,PCHAR key,UCHAR conn);
extern void xJSON_putMAC(xJSON * msg,PCHAR key,UCHAR conn);
extern void xJSON_putVendorID(xJSON * msg,PCHAR key,UCHAR conn);
extern void xJSON_putVerCode(xJSON * msg,PCHAR key,UCHAR conn);
extern void xJSON_putDeviceID(xJSON * msg,PCHAR key,UCHAR conn);
extern void xJSON_putProductName(xJSON * msg,PCHAR key,UCHAR conn);
extern void xJSON_putVendorName(xJSON * msg,PCHAR key,UCHAR conn);
extern void xJSON_putSerialCode(xJSON * msg,PCHAR key,UCHAR conn);
extern void xJSON_putMCUVerCode(xJSON * msg,PCHAR key,UCHAR conn);
extern void xJSON_putAttribute(xJSON * msg,INT32 attrbute,UCHAR conn);
#endif
