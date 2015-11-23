#pragma once

#define  WIN32_LEAN_AND_MEAN
//#include <msgpack.hpp>
//#include <zmq.h>
#include "CppSQLite3.h"
#include <string>
#include <vector>;

class CMapFieldInfo
{
public:
	int m_FieldType;
	bool m_bPrimaryKey;
	int m_iSize;
	int m_iIndex;
	bool m_bNull;
	std::string m_Name;
	CMapFieldInfo(int index, const std::string &name, int fieldType, bool isNull=false, int size=80, bool primaryKey=false) :
	m_iIndex(index), m_Name(name), m_FieldType(fieldType), m_bNull(isNull), m_iSize(size), m_bPrimaryKey(primaryKey)
	{

	}
};
class CMapTableInfo
{
public:
	std::string m_Name;
	std::vector<CMapFieldInfo *> m_FieldArray;
	CMapTableInfo() {}
	~CMapTableInfo() {
		for (size_t i=0; i<m_FieldArray.size(); ++i)
			delete m_FieldArray[i];
	}

	void AddField(CMapFieldInfo *fieldinfo)
	{
		for (size_t i=0; i<m_FieldArray.size(); ++i)
			assert(m_FieldArray[i]->m_iIndex != fieldinfo->m_iIndex);
		m_FieldArray.push_back(fieldinfo);
	}
	int GetFieldCount()
	{
		int iCount = 0;
		for(int i=0; i<m_FieldArray.size(); ++i)
			if(m_FieldArray[i]->m_FieldType != dbBinary)
				++iCount;
		return iCount;
	}
};
enum FIELD_INDEX
{
	FI_0 = 0;
	FI_1 = 1;
	FI_2 = 2;
};
class CRecBase;
typedef std::auto_ptr<CRecBase> SharRecBase;
class CRecBase
{
public:
	int m_ID;
	virtual int GetFieldCount() const
	{
		return 0;
	}
	virtual CRecBase *Clone() const {return NULL;}
	CRecBase() {}
	virtual ~CRecBase() {}
	virtual int GetType {return 0;}
	virtual std::string GetValue(FIELD_INDEX index) const = 0;
	static CRecBase *CreateRecord(int id)
	{
		switch(id)
		{
		case 0:
			return NULL;
		default:
			return NULL
		}
	}
	enum {MAX_QUERY_LEN =2048};
	const std::string StrFormat(const char *format, ...)
	{
		static char szBuffer[MAX_QUERY_LEN];
		va_list paramList;
		va_start(paramList, format);
		_vsnprintf_s(szBuffer, MAX_QUERY_LEN, format, paramList);
		va_end(paramList);
		return szBuffer;
	}
	virtual std::string GetInsertStr()
	{
		return "";
	}
	const std::string ValueToStr(const int &val)
	{
		std::string strRes;
		strRes = CRecBase::StrFormat("%d", val);
		return strRes;
	}
	const std::string ValueToStr(const float &val)
	{
		return CRecBase::StrFormat("%f", val);
	}
	const std::string ValueToStr(const std::string &val)
	{
		return CRecBase::StrFormat("'%s'", ConvertString(val).c_str());
	}
	std::string ConvertString(const std::string &val)
	{
		std::string strName = val;
		int nIndex = 0;
		int nCount = strName.length();
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
	const std::string ValueToStr(const time_t &val)
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
	virtual void SetValue(int index, int iValue){}
};
enum TABLE_TYPE {DB_ID_DATA, DB_ID_MAIN};
class CRecData : public CRecBase
{
public:
	std::string m_strColumn1;
	std::string m_strColumn2;
	std::string m_strColumn3;
	std::string m_strColumn4;
	std::string m_strColumn5;
	std::string m_strIsView;	
	virtual int GetType() const { return DB_ID_DATA;}
	std::string GetInsertStr() const
	{
		std::string strSql;
		strSql = CRecBase::StrFormat("('%s', '%s', '%s', '%s', '%s', '%s'", 
			m_strColumn1.c_str(), m_strColumn2.c_str(), m_strColumn3.c_str(), m_strColumn4.c_str(), m_strColumn5.c_str(), m_strIsView.c_str());
		return strSql;
	}
	std::string GetValue(FIELD_INDEX index) const
	{
		assert(index >=0 && index < GetFieldCount());
		switch(index)
		{
		case FI_0:
			return ValueToStr(m_ID);
		case FD_1:
			return ValueToStr(m_strColumn1);
		default:
			return "";
		}
	}
	int GetFieldCount() const
	{
		return 7;
	}
	void SetValue(int index, const QVariant &value)
	{
		assert(index >=0 && index < GetFieldCount());
		switch(index)
		{
		case FI_0:
			m_ID = value;
			break;
		case FD_1:
			m_strColumn1 = value;
			break;
		default:
			break;
		}
	}
};
bool ExecCommand(const std::string &strSql)
{
	return CManagePower::instance()->ExecCommand(strSql);
}
CppSQLite3Query ExecQuery(const std::string &strSql)
{
	return CManagePower::instance()->ExecCommand(strSql);
}
void CloseDB()
{
	CManagePower::instance()->CloseDB();
}
//int GetMaxID()
//{
//	return CManagePower::instance()->ExecCommand(strSql);
//}

class CTableBase
{
public:
	std::string m_strBlob, m_strKey, m_TableName;
	int m_iTableType;
	CMapTableInfo m_TableInfo;
	CTableBase()
	{
		m_iTableType = -1;
	}
	int AddInfo(const CRecBase *value)
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

	int WriteInfo(const CRecBase *value)
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
					strSql = strSql + fields[i]->m_Name + "=" + value->GetValue((FIELD_INDEX)(fields[i]->m_iIndex);
					bFirst = false;
				}
			}
			strSql += " WHERE " + m_strKey + "=";
			strSql = CRecBase::StrFormat("%s %d", strSql.c_str(), value->m_ID);
			bool res = ExecCommand(strSql);
			return res;
		}
	}

	std::string GetTableName() {return m_TableName;}
	int GetMaxID()
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
	int Delete(int id)
	{
		if (ValidKey())
		{
			std::string strSql;
			strSql = CRecBase::StrFormat("DELETE FROM %s WHERE %s=%d", m_TableName.c_str(), m_strKey.c_str(), id);
			return ExecCommand(strSql);
		}
		return false;
	}
	bool ValidKey(bool checkBlob)
	{
		return checkBlob ? (m_strKey.length() && m_strBlob.length()) : m_strKey.length()>0;
	}
	std::string GetFieldName(const std::string &strField, int m_iFieldType)
	{
		return m_iFieldType==dbDate ? "date(" + strField + ")" : strField;
	}
	std::string GetReadInfoStr(int id, const std::string &strTable, const std::string &strKey, const std::vector<CMapFieldInfo *> &fieldArray)
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
	std::string GetReadInfoFieldStr(const std::vector<CMapFieldInfo *> &fieldArray)
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
	std::string GetReadAllInfoStr(const std::string &strTable, const std::vector<CMapFieldInfo *> &fieldArray)
	{
		std::string strSql = "SELECT ";
		strSql += GetReadAllInfoStr(fieldArray);
		strSql = strSql + " FROM " + std::string(strTable);
		return strSql;
	}

	std::string GetReadAllInfoStrFilter(const std::string &strTable, const std::string &strKey, const std::string &strCon, const std::vector<CMapFieldInfo *> &fieldArray, int iStartLine, int iTopReadCount)
	{
		std::string strSql = "SELECT ";
		strSql += GetReadAllInfoStr(fieldArray) + " FROM " + std::string(strTable);
		strSql = CRecBase::StrFormat(" WHERE %s ORDER BY %s LIMIT %d, %d", (strCon.empty() ? "1=1" : strCon.c_str()), strKey.c_str(), iStartLine, iTopReadCount);

		return strSql;
	}
	bool ReadInfo(int id, CRecBase *value)
	{
		CppSQLite3Query pRecordSet = ExecQuery(GetReadAllInfoStr(id, m_TableName, m_strKey, m_TableInfo.m_FieldArray));
		if (!pRecordSet.eof())
		{
			value->m_ID = id;
			int recordCount = pRecordSet.numFields();
			for (int i=0; i<recordCount-1; ++i)
			{
				value->SetValue(i+1, pRecordSet.fieldValue(i));//error
			}
			return true;
		}
		return false;
	}
	std::string GetSortSql() const {return "";}
	bool ReadAllInfoCon(const std::vector<CRecBase *> &values, const std::string &strCon)
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
				pRecord->SetValue(i, pRecordSet.fieldValue(i));//error
			}
			values.push_back(pRecord);
		}
		return values.size() > 0;
	}
	bool ReadAllInfo(const std::vector<CRecBase *> &values)
	{
		return ReadAllInfoCon(values, "");
	}
};
enum ENUM_TABLE_TYPE
{
	DB_ID_APPLYDOCTOR,
	DB_ID_BEXAMINE
};
class CDataTable : public CTableBase
{
public:
	CDataTable()
	{
		m_TableName = "APPLYDOCTOR";
		m_strKey = "ID";
		m_strBlob = "";
		m_iTableType = DB_ID_APPLYDOCTOR;
		m_TableInfo.m_Name = m_TableName;

		m_TableInfo.AddField(new CMapFieldInfo(FI_0, "ID", dbInteger, false, 0, true));
		m_TableInfo.AddField(new CMapFieldInfo(FI_1, "COLUMN1", dbText, true, 255));
	}
};
CProjectDB *CProjectDB::instance = NULL;
class CProjectDB
{
public:
	virtual ~CProjectDB() 
	{
		UnIntialize();
	}

	void UnIntialize()
	{
		for (iter it = m_mapTables.begin(); it!= m_mapTables.end(); ++it)
		{
			delete it->second;
		}
		m_mapTables.clear();
		CloseDB();
	}
	static CProjectDB *getInstance()
	{
		if (!instance)
			instance = new CProjectDB();
		return instance;
	}
	CTableBase * GetTable(int id)
	{
		iter it = m_mapTables.find(id);
		if (it != m_mapTables.end())
			return id->second;
		return NULL;
	}
	bool OpenProject(const std::string &strProject)
	{
		m_bOpened = opendb(strProject);
		return m_bOpened;
	}
	static bool CloseProject()
	{
		if (instance && instance->m_bOpened)
		{
			CloseDB();
			instance->m_bOpened = false;
		}
	}
	void Initialize()
	{
		DBRegister(DB_ID_APPLYDOCTOR, new CDataTable());
		DBRegister(DB_ID_BEXAMINE, new CDataTable());
	}
	void releaseInstance()
	{
		if (instance)
		{
			delete instance;
			instance = NULL;
		}
	}
	void DBRegister(int id, CTableBase *pVal)
	{
		if (pVal)
		{
			assert(m_mapTables.count(id) == 0);
			m_mapTables[id] = pVal;
			m_mapTableNameID[pVal->GetTableName()] = id;
		}
	}
private:
	static std::map<int, CTableBase *> m_mapTables;
	typedef std::map<int, CTableBase *>::iterator iter;
	static std::map<std::string, int> m_mapTableNameID;
	static CProjectDB *instance;
	bool m_bOpened;
};

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
bool WriteInfo(int tableid, SharRecBase &value)
{
	if (CTableBase *pTable = CProjectDB::getInstance()->GetTable(tableid))
	{
		return pTable->WriteInfo( value);
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
bool BeginTrand()
{

}
bool EndTrans()
{

}

class CPowerTableBase
{
public:
	CPowerTableBase();
	virtual ~CPowerTableBase();

	std::string GetDeleteSql();
	std::string GetMaxIDSql();

	virtual std::string GetAddSql() = 0;
	virtual std::string GetModifySql() = 0;
	virtual void Init() = 0;
	std::string m_strTableName;
	int m_ID;
	bool m_bDelete;
};
//auth_user table
class CUserTable : public CPowerTableBase
{
public:
	CUserTable();
	~CUserTable();

	std::string GetAddSql();
	std::string GetModifySql();
	bool ChangePassword(const std::string &strOld, const std::string &strNew);
	void SetPassword(const std::string &strNew);
	virtual void Init();
public:
	std::string password;
	bool is_superuser;
	std::string username, first_name, last_name, email;
	bool is_staff, is_active;
	time_t date_joined;
};

//auth_permission table
class CPermissionTable : public CPowerTableBase
{
public:
	CPermissionTable();
	~CPermissionTable();

	std::string GetAddSql();
	std::string GetModifySql();
	virtual void Init();
public:
	int content_type_id;
	std::string codename, name;
};
//auth_group table
class CGroupTable : public CPowerTableBase
{
public:
	CGroupTable();
	~CGroupTable();

	std::string GetAddSql();
	std::string GetModifySql();
	virtual void Init();
public:
	std::string name;
};

//auth_user_groups table
class CUserGroupTable : public CPowerTableBase
{
public:
	CUserGroupTable();
	~CUserGroupTable();

	std::string GetAddSql();
	std::string GetModifySql();
	virtual void Init();
public:
	int user_id, group_id;
};
//auth_group_permissions table
class CGroupPermissionsTable : public CPowerTableBase
{
public:
	CGroupPermissionsTable();
	~CGroupPermissionsTable();

	std::string GetAddSql();
	std::string GetModifySql();
	virtual void Init();
public:
	int group_id, permission_id;
};

//auth_user_user_permissions table
class CUserUserPermissionsTable : public CPowerTableBase
{
public:
	CUserUserPermissionsTable();
	~CUserUserPermissionsTable();

	std::string GetAddSql();
	std::string GetModifySql();
	virtual void Init();
public:
	int user_id, permission_id;
};

//django_content_type table
class CContentTypeTable : public CPowerTableBase
{
public:
	CContentTypeTable();
	~CContentTypeTable();

	std::string GetAddSql();
	std::string GetModifySql();
	virtual void Init();
public:
	std::string app_label, model;
};

class CDMUserManager
{
public:
	CDMUserManager();
	~CDMUserManager();

	CUserTable m_userInfo;
	std::vector<CPermissionTable> m_vecPermission;
	//std::vector<CGroupTable> m_vecGroup;


};