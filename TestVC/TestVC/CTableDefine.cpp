#include "stdafx.h"

#include "CTableDefine.h"
#define  WIN32_LEAN_AND_MEAN
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include "commonfunc.h"

#pragma warning (disable: 4996)
#define bExport true

using namespace std;
//////////////
bool ExecCommand(const std::string &strSql);
CppSQLite3Query ExecQuery(const std::string &strSql);
void CloseDB();

//CMapFieldInfo
CMapFieldInfo::CMapFieldInfo(int index, const std::string &name, int fieldType, bool isNull, int size, bool primaryKey) :
    m_iIndex(index), m_Name(name), m_FieldType(fieldType), m_bNull(isNull), m_iSize(size), m_bPrimaryKey(primaryKey)
{
}

//CMapTableInfo
CMapTableInfo::CMapTableInfo() {}
CMapTableInfo::~CMapTableInfo()
{
    for (size_t i=0; i<m_FieldArray.size(); ++i)
        delete m_FieldArray[i];
}

void CMapTableInfo::AddField(CMapFieldInfo *fieldinfo)
{
    for (size_t i=0; i<m_FieldArray.size(); ++i)
        assert(m_FieldArray[i]->m_iIndex != fieldinfo->m_iIndex);
    m_FieldArray.push_back(fieldinfo);
}
int CMapTableInfo::GetFieldCount()
{
    int iCount = 0;
    for(size_t i=0; i<m_FieldArray.size(); ++i)
        if(m_FieldArray[i]->m_FieldType != dbBinary)
            ++iCount;
    return iCount;
}

CProjectDB *CProjectDB::instance = NULL;

//CRecBase
int CRecBase::GetFieldCount() const
{
    return 0;
}
CRecBase *CRecBase::Clone() const
{
    return NULL;
}
CRecBase::CRecBase() {}
CRecBase::~CRecBase() {}
int CRecBase::GetType()
{
    return 0;
}
CRecBase *CRecBase::CreateRecord(int id)
{
    switch(id)
    {
    case DB_ID_APPLYDOCTOR:
	case DB_ID_BEXAMINE:
        return new CRecData();
    default:
        return NULL;
    }
}
const std::string CRecBase::StrFormat(const char *format, ...)
{
    static char szBuffer[MAX_QUERY_LEN];
    va_list paramList;
    va_start(paramList, format);
    _vsnprintf_s(szBuffer, MAX_QUERY_LEN, format, paramList);
    va_end(paramList);
    return szBuffer;
}
std::string CRecBase::GetInsertStr() const
{
    return "";
}
const std::string CRecBase::ValueToStr(const int &val)
{
    std::string strRes;
    strRes = CRecBase::StrFormat("%d", val);
    return strRes;
}
const std::string CRecBase::ValueToStr(const float &val)
{
    return CRecBase::StrFormat("%f", val);
}
const std::string CRecBase::ValueToStr(const std::string &val)
{
    return CRecBase::StrFormat("'%s'", ConvertString(val).c_str());
}
std::string CRecBase::ConvertString(const std::string &val)
{
    std::string strName = val;
    size_t nIndex = 0;
    size_t nCount = strName.length();
    while(nIndex < nCount)
    {
        if (strName[nIndex] == '\'')
        {
            strName.insert(nIndex, "\'");
            ++nCount;
            ++nIndex;
        }
        ++nIndex;
    }
    return strName;
}
const std::string CRecBase::ValueToStr(const time_t &val)
{
    static char strTime[20];
    if (-1 != val)
    {
        tm ptr;
        localtime_s(&ptr, &val);
        strftime(strTime, 20, "'%Y-%m-%d'", &ptr);
    }
    return val!=-1 ? strTime : "''";
}
void CRecBase::SetValue(int index, CppSQLite3Query &pRecordSet) {}

//CRecData
CRecData::CRecData() {}
int CRecData::GetType() const
{
    return DB_ID_DATA;
}
std::string CRecData::GetInsertStr() const
{
    std::string strSql;
    strSql = CRecBase::StrFormat("('%s', '%s', '%s', '%s', '%s', '%s'",
                                 m_strColumn1.c_str(), m_strColumn2.c_str(), m_strColumn3.c_str(), m_strColumn4.c_str(), m_strColumn5.c_str(), m_strIsView.c_str());
    return strSql;
}
std::string CRecData::GetValue(FIELD_INDEX index) const
{
    assert(index >=0 && index < GetFieldCount());
    switch(index)
    {
    case FI_0:
        return ValueToStr(m_ID);
    case FI_1:
        return ValueToStr(m_strColumn1);
    case FI_2:
        return ValueToStr(m_strColumn2);
    case FI_3:
        return ValueToStr(m_strColumn3);
    case FI_4:
        return ValueToStr(m_strColumn4);
    case FI_5:
        return ValueToStr(m_strColumn5);
    case FI_6:
        return ValueToStr(m_strIsView);
    default:
        return "";
    }
}
int CRecData::GetFieldCount() const
{
    return 7;
}
void CRecData::SetValue(int index, CppSQLite3Query &pRecordSet)
{
    assert(index >=0 && index < GetFieldCount());
    switch(index)
    {
    case FI_0:
        m_ID = pRecordSet.getIntField(index);
        break;
    case FI_1:
		{
			m_strColumn1 = ConvertSqliteString(pRecordSet.getStringField(index));
		}
        break;
    case FI_2:
        m_strColumn2 = ConvertSqliteString(pRecordSet.getStringField(index));
        break;
    case FI_3:
        m_strColumn3 = ConvertSqliteString(pRecordSet.getStringField(index));
        break;
    case FI_4:
        m_strColumn4 = ConvertSqliteString(pRecordSet.getStringField(index));
        break;
    case FI_5:
        m_strColumn5 = ConvertSqliteString(pRecordSet.getStringField(index));
        break;
    default:
        break;
    }
}

bool ExecCommand(const std::string &strSql)
{
    return CManageDB::instance()->ExecCommand(strSql);
}
CppSQLite3Query ExecQuery(const std::string &strSql)
{
    return CManageDB::instance()->ExecQuery(strSql);
}
void CloseDB()
{
    CManageDB::instance()->CloseDB();
}

//CTableBase
CTableBase::CTableBase()
{
    m_iTableType = -1;
}
int CTableBase::AddInfo(const CRecBase *value)
{
    std::string strSql = "INSERT INTO " + m_TableName + " (";
    bool bFirst = true;
    const std::vector<CMapFieldInfo *> &fields = m_TableInfo.m_FieldArray;
    for(size_t i=0; i<fields.size(); ++i)
    {
        if (!fields[i]->m_bPrimaryKey && fields[i]->m_FieldType != dbBinary)
        {
            if (!bFirst)
            {
                strSql += ",";
            }
            strSql += fields[i]->m_Name;
            bFirst = false;
        }
    }
    strSql += ") VALUES " + value->GetInsertStr();
    int ID = 0;
    bool res = ExecCommand(strSql);
    if (res)
        ID = GetMaxID();
    return ID;
}

bool CTableBase::WriteInfo(const CRecBase * value)
{
    if (value->m_ID > 0)
    {
        std::string strSql = "UPDATE " + m_TableName + " SET ";
        bool bFirst = true;
        const std::vector<CMapFieldInfo *> &fields = m_TableInfo.m_FieldArray;
        for(size_t i=0; i<fields.size(); ++i)
        {
            if (!fields[i]->m_bPrimaryKey && fields[i]->m_FieldType != dbBinary)
            {
                if (!bFirst)
                {
                    strSql += ",";
                }
                strSql = strSql + fields[i]->m_Name + "=" + value->GetValue((FIELD_INDEX)(fields[i]->m_iIndex));
                bFirst = false;
            }
        }
        strSql += " WHERE " + m_strKey + "=";
        strSql = CRecBase::StrFormat("%s %d", strSql.c_str(), value->m_ID);
        bool res = ExecCommand(strSql);
        return res;
    }
	return false;
}

std::string CTableBase::GetTableName()
{
    return m_TableName;
}
int CTableBase::GetMaxID()
{
    int ID = -1;
    if (ValidKey())
    {
        std::string strSql;
        strSql = CRecBase::StrFormat("SELECT MAX(%s) AS MaxID FROM %s", m_strKey.c_str(), m_TableName.c_str());
        CppSQLite3Query pRecordSet = ExecQuery(strSql);
        if (!pRecordSet.eof())
            ID = pRecordSet.getIntField(0, 0);
    }
    return ID;
}
bool CTableBase::Delete(int id)
{
    if (ValidKey())
    {
        std::string strSql;
        strSql = CRecBase::StrFormat("DELETE FROM %s WHERE %s=%d", m_TableName.c_str(), m_strKey.c_str(), id);
        return ExecCommand(strSql);
    }
    return false;
}
bool CTableBase::ValidKey(bool checkBlob)
{
    return checkBlob ? (m_strKey.length() && m_strBlob.length()) : m_strKey.length()>0;
}
std::string CTableBase::GetFieldName(const std::string &strField, int m_iFieldType)
{
    return m_iFieldType==dbDate ? "date(" + strField + ")" : strField;
}
std::string CTableBase::GetReadInfoStr(int id, const std::string &strTable, const std::string &strKey, const std::vector<CMapFieldInfo *> &fieldArray)
{
    std::string strSql = "SELECT ";
    bool bFirst = true;
    for(size_t i=0; i<fieldArray.size(); ++i)
    {
        if (!fieldArray[i]->m_bPrimaryKey && fieldArray[i]->m_FieldType != dbBinary)
        {
            if (!bFirst)
            {
                strSql += ",";
            }
            strSql = strSql + GetFieldName(fieldArray[i]->m_Name, fieldArray[i]->m_FieldType);
            bFirst = false;
        }
    }
    strSql += " FROM " + strTable + " WHERE " + strKey + "=";
    strSql = CRecBase::StrFormat("%s %d", strSql.c_str(), id);
    return strSql;
}
std::string CTableBase::GetReadInfoFieldStr(const std::vector<CMapFieldInfo *> &fieldArray)
{
    std::string strSql;
    bool bFirst = true;
    for(size_t i=0; i<fieldArray.size(); ++i)
    {
        if (fieldArray[i]->m_FieldType != dbBinary)
        {
            if (!bFirst)
            {
                strSql += ",";
            }
            strSql = strSql + GetFieldName(fieldArray[i]->m_Name, fieldArray[i]->m_FieldType);
            bFirst = false;
        }
    }
    return strSql;
}
std::string CTableBase::GetReadAllInfoStr(const std::string &strTable, const std::vector<CMapFieldInfo *> &fieldArray)
{
    std::string strSql = "SELECT ";
    strSql += GetReadInfoFieldStr(fieldArray);
    strSql = strSql + " FROM " + std::string(strTable);
    return strSql;
}

std::string CTableBase::GetReadAllInfoStrFilter(const std::string &strTable, const std::string &strKey, const std::string &strCon, const std::vector<CMapFieldInfo *> &fieldArray, int iStartLine, int iTopReadCount)
{
    std::string strSql = "SELECT ";
    strSql += GetReadInfoFieldStr(fieldArray) + " FROM " + std::string(strTable);
    strSql = CRecBase::StrFormat(" WHERE %s ORDER BY %s LIMIT %d, %d", (strCon.empty() ? "1=1" : strCon.c_str()), strKey.c_str(), iStartLine, iTopReadCount);
    return strSql;
}
bool CTableBase::ReadInfo(int id, SharRecBase value)
{
	value = SharRecBase(CRecBase::CreateRecord(id));
    CppSQLite3Query pRecordSet = ExecQuery(GetReadInfoStr(id, m_TableName, m_strKey, m_TableInfo.m_FieldArray));
    if (!pRecordSet.eof())
    {
        value->m_ID = id;
        int recordCount = pRecordSet.numFields();
        for (int i=0; i<recordCount-1; ++i)
        {
            value->SetValue(i+1, pRecordSet);//error
        }
        return true;
    }
    return false;
}
std::string CTableBase::GetSortSql() const
{
    return "";
}
bool CTableBase::ReadAllInfoCon(std::vector<SharRecBase> &values, const std::string &strCon)
{
    std::string strReadAllSql = GetReadAllInfoStr(m_TableName, m_TableInfo.m_FieldArray) + GetSortSql();
    strReadAllSql = strCon.empty() ? strReadAllSql : strReadAllSql + " WHERE " + strCon;
    CppSQLite3Query pRecordSet = ExecQuery(strReadAllSql);
    int recordCount = m_TableInfo.GetFieldCount();
    while (!pRecordSet.eof())
    {
        CRecBase *pRecord = CRecBase::CreateRecord(m_iTableType);
        for (int i=0; i<recordCount; ++i)
        {
            pRecord->SetValue(i, pRecordSet);//error
        }
        values.push_back(SharRecBase(pRecord));
		pRecordSet.nextRow();
    }
    return values.size() > 0;
}
bool CTableBase::ReadAllInfo(std::vector<SharRecBase> &values)
{
    return ReadAllInfoCon(values, "");
}
//CDataTable
CDataTable::CDataTable(const std::string &strTableName)
{
    m_TableName = strTableName;//;
    m_strKey = "ID";
    m_strBlob = "";
    m_iTableType = DB_ID_APPLYDOCTOR;
    m_TableInfo.m_Name = m_TableName;
    m_TableInfo.AddField(new CMapFieldInfo(FI_0, "ID", dbInteger, false, 0, true));
    m_TableInfo.AddField(new CMapFieldInfo(FI_1, "COLUMN1", dbText, true, 255));
    m_TableInfo.AddField(new CMapFieldInfo(FI_2, "COLUMN2", dbText, true, 255));
    m_TableInfo.AddField(new CMapFieldInfo(FI_3, "COLUMN3", dbText, true, 255));
    m_TableInfo.AddField(new CMapFieldInfo(FI_4, "COLUMN4", dbText, true, 255));
    m_TableInfo.AddField(new CMapFieldInfo(FI_5, "COLUMN5", dbText, true, 255));
    m_TableInfo.AddField(new CMapFieldInfo(FI_6, "ISVIEW", dbText, true, 255));
}

//CProjectDB
std::map<int, CTableBase *> CProjectDB::m_mapTables;
std::map<std::string, int> CProjectDB::m_mapTableNameID;
CProjectDB::CProjectDB()
{
	Initialize();
}
CProjectDB:: ~CProjectDB()
{
    UnIntialize();
}

void CProjectDB::UnIntialize()
{
    for (iter it = m_mapTables.begin(); it!= m_mapTables.end(); ++it)
    {
        delete it->second;
    }
    m_mapTables.clear();
    CloseDB();
}
CProjectDB *CProjectDB::getInstance()
{
    if (!instance)
        instance = new CProjectDB();
    return instance;
}
CTableBase * CProjectDB::GetTable(int id)
{
    iter it = m_mapTables.find(id);
    if (it != m_mapTables.end())
        return it->second;
    return NULL;
}
bool CProjectDB::OpenProject(const std::string &strProject)
{
    m_bOpened = CManageDB::instance()->ConnectDB(strProject);
    return m_bOpened;
}
void CProjectDB::CloseProject()
{
    if (instance && instance->m_bOpened)
    {
        CloseDB();
        instance->m_bOpened = false;
    }
}
void CProjectDB::Initialize()
{
    DBRegister(DB_ID_APPLYDOCTOR, new CDataTable("APPLYDOCTOR"));
    DBRegister(DB_ID_BEXAMINE, new CDataTable("BEXAMINE"));
}
void CProjectDB::releaseInstance()
{
    if (instance)
    {
        delete instance;
        instance = NULL;
    }
}
void CProjectDB::DBRegister(int id, CTableBase *pVal)
{
    if (pVal)
    {
        assert(m_mapTables.count(id) == 0);
        m_mapTables[id] = pVal;
        m_mapTableNameID[pVal->GetTableName()] = id;
    }
}

bool OpenProject(const std::string &strName)
{
    return CProjectDB::getInstance()->OpenProject(strName);
}
void CloseProject()
{
    CProjectDB::CloseProject();
}
bool ReadInfo(int tableid, int recordid, SharRecBase &value)
{
    if (CTableBase *pTable = CProjectDB::getInstance()->GetTable(tableid))
    {
        return pTable->ReadInfo(recordid, value);
    }
    return false;
}
bool WriteInfo(int tableid, CRecBase *value)
{
    if (CTableBase *pTable = CProjectDB::getInstance()->GetTable(tableid))
    {
        return pTable->WriteInfo( value);
    }
    return false;
}
int AddInfo(int tableid, CRecBase *value)
{
	if (CTableBase *pTable = CProjectDB::getInstance()->GetTable(tableid))
	{
		return pTable->AddInfo( value);
	}
	return false;
}
void ReleaseInstance()
{
    CProjectDB::releaseInstance();
}

bool ReadAllInfo(int tableid, std::vector<SharRecBase> &values)
{
    if (CTableBase *pTable = CProjectDB::getInstance()->GetTable(tableid))
    {
        return pTable->ReadAllInfo(values);
    }
    return false;
}
bool ReadAllInfoCon(int tableid, std::vector<SharRecBase> &values, const std::string &strCon)
{
    if (CTableBase *pTable = CProjectDB::getInstance()->GetTable(tableid))
    {
        return pTable->ReadAllInfoCon(values, strCon);
    }
    return false;
}
bool Delete(int tableid, int id)
{
    if (CTableBase *pTable = CProjectDB::getInstance()->GetTable(tableid))
    {
        return pTable->Delete(id);
    }
    return false;
}
bool BeginTrans()
{
    return CManageDB::instance()->BeginTrans();
}
bool CommitTrans()
{
    return CManageDB::instance()->CommitTrans();
}
bool RollbackTrans()
{
    return CManageDB::instance()->RollbackTrans();
}