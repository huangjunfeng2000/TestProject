#pragma once

enum FIELD_INDEX
{
	FI_0 = 0,
	FI_1 = 1,
	FI_2 = 2,
	FI_3 = 3,
	FI_4 = 4,
	FI_5 = 5,
	FI_6 = 6,
	FI_7 = 7,
	FI_8 = 8,
	FI_9 = 9,
	FI_10 = 10,
	FI_11 = 11,
	FI_12 = 12,

};
enum TABLE_TYPE {DB_ID_DATA, DB_ID_MAIN};
enum ENUM_TABLE_TYPE
{
	DB_ID_APPLYDOCTOR,
	DB_ID_BEXAMINE
};

typedef enum DataTypeEnum
{	dbBoolean	= 1,
dbByte	= 2,
dbInteger	= 3,
dbLong	= 4,
dbCurrency	= 5,
dbSingle	= 6,
dbDouble	= 7,
dbDate	= 8,
dbBinary	= 9,
dbText	= 10,
dbLongBinary	= 11,
dbMemo	= 12,
dbGUID	= 15,
dbBigInt	= 16,
dbVarBinary	= 17,
dbChar	= 18,
dbNumeric	= 19,
dbDecimal	= 20,
dbFloat	= 21,
dbTime	= 22,
dbTimeStamp	= 23
}	DataTypeEnum;


