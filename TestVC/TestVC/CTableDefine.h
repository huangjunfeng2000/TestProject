#pragma once

#define  WIN32_LEAN_AND_MEAN
//#include <msgpack.hpp>
//#include <zmq.h>
#include "CppSQLite3.h"
#include "dbtypes.h"
#include <string>
#include <vector>
#include "CManageDB.h"
#include <map>
#include <wchar.h>
#include <assert.h>
#include <boost/shared_ptr.hpp>

class CRecBase;
typedef boost::shared_ptr<CRecBase> SharRecBase;



bool OpenProject(const std::string &strName);
void CloseProject();
bool ReadInfo(int tableid, int recordid, SharRecBase &value);
bool WriteInfo(int tableid, CRecBase *value);
int AddInfo(int tableid, CRecBase *value);
void ReleaseInstance();
bool ReadAllInfo(int tableid, std::vector<SharRecBase> &values);
bool ReadAllInfoCon(int tableid, std::vector<SharRecBase> &values, const std::string &strCon);
bool Delete(int tableid, int id);
bool BeginTrans();
bool CommitTrans();
bool RollbackTrans();


class CMapFieldInfo
{
public:
	int m_FieldType;
	bool m_bPrimaryKey;
	int m_iSize;
	int m_iIndex;
	bool m_bNull;
	std::string m_Name;
	CMapFieldInfo(int index, const std::string &name, int fieldType, bool isNull=false, int size=80, bool primaryKey=false);
};

class CMapTableInfo
{
public:
	std::string m_Name;
	std::vector<CMapFieldInfo *> m_FieldArray;
	CMapTableInfo();
	~CMapTableInfo();
	void AddField(CMapFieldInfo *fieldinfo);
	int GetFieldCount();
	
};

//class CRecData;

class CRecBase
{
public:
	int m_ID;
	virtual int GetFieldCount() const;
	virtual CRecBase *Clone() const;
	CRecBase();
	virtual ~CRecBase();
	virtual int GetType();
	virtual std::string GetValue(FIELD_INDEX index) const = 0;
	static CRecBase *CreateRecord(int id);
	enum {MAX_QUERY_LEN =2048};
	static const std::string StrFormat(const char *format, ...);
	virtual std::string GetInsertStr() const;
	static const std::string ValueToStr(const int &val);
	static const std::string ValueToStr(const float &val);
	static const std::string ValueToStr(const std::string &val);
	static std::string ConvertString(const std::string &val);
	static const std::string ValueToStr(const time_t &val);
	virtual void SetValue(int index, CppSQLite3Query &pRecordSet);
};

class CRecData : public CRecBase
{
public:
	std::string m_strColumn1;
	std::string m_strColumn2;
	std::string m_strColumn3;
	std::string m_strColumn4;
	std::string m_strColumn5;
	std::string m_strIsView;	
	CRecData();
	virtual int GetType() const;
	std::string GetInsertStr() const;
	std::string GetValue(FIELD_INDEX index) const;
	int GetFieldCount() const;
	void SetValue(int index, CppSQLite3Query &pRecordSet);
};

class CTableBase
{
public:
	std::string m_strBlob, m_strKey, m_TableName;
	int m_iTableType;
	CMapTableInfo m_TableInfo;
	CTableBase();
	int AddInfo(const CRecBase *value);
	bool WriteInfo(const CRecBase *value);
	std::string GetTableName();
	int GetMaxID();
	bool Delete(int id);
	bool ValidKey(bool checkBlob = false);
	std::string GetFieldName(const std::string &strField, int m_iFieldType);
	std::string GetReadInfoStr(int id, const std::string &strTable, const std::string &strKey, const std::vector<CMapFieldInfo *> &fieldArray);
	std::string GetReadInfoFieldStr(const std::vector<CMapFieldInfo *> &fieldArray);
	std::string GetReadAllInfoStr(const std::string &strTable, const std::vector<CMapFieldInfo *> &fieldArray);
	std::string GetReadAllInfoStrFilter(const std::string &strTable, const std::string &strKey, const std::string &strCon, const std::vector<CMapFieldInfo *> &fieldArray, int iStartLine, int iTopReadCount);
	bool ReadInfo(int id, SharRecBase value);
	std::string GetSortSql() const;
	bool ReadAllInfoCon(std::vector<SharRecBase> &values, const std::string &strCon);
	bool ReadAllInfo(std::vector<SharRecBase> &values);
};

class CDataTable : public CTableBase
{
public:
	CDataTable(const std::string &strTableName);
};

class CProjectDB
{
public:
	CProjectDB();
	virtual ~CProjectDB();
	void UnIntialize();
	static CProjectDB *getInstance();
	CTableBase * GetTable(int id);
	bool OpenProject(const std::string &strProject);
	static void CloseProject();
	void Initialize();
	static void releaseInstance();
	void DBRegister(int id, CTableBase *pVal);
private:
	static std::map<int, CTableBase *> m_mapTables;
	typedef std::map<int, CTableBase *>::iterator iter;
	static std::map<std::string, int> m_mapTableNameID;
	static CProjectDB *instance;
	bool m_bOpened;
};

//class CPowerTableBase
//{
//public:
//	CPowerTableBase();
//	virtual ~CPowerTableBase();
//
//	std::string GetDeleteSql();
//	std::string GetMaxIDSql();
//
//	virtual std::string GetAddSql() = 0;
//	virtual std::string GetModifySql() = 0;
//	virtual void Init() = 0;
//	std::string m_strTableName;
//	int m_ID;
//	bool m_bDelete;
//};
////auth_user table
//class CUserTable : public CPowerTableBase
//{
//public:
//	CUserTable();
//	~CUserTable();
//
//	std::string GetAddSql();
//	std::string GetModifySql();
//	bool ChangePassword(const std::string &strOld, const std::string &strNew);
//	void SetPassword(const std::string &strNew);
//	virtual void Init();
//public:
//	std::string password;
//	bool is_superuser;
//	std::string username, first_name, last_name, email;
//	bool is_staff, is_active;
//	time_t date_joined;
//};
//
////auth_permission table
//class CPermissionTable : public CPowerTableBase
//{
//public:
//	CPermissionTable();
//	~CPermissionTable();
//
//	std::string GetAddSql();
//	std::string GetModifySql();
//	virtual void Init();
//public:
//	int content_type_id;
//	std::string codename, name;
//};
////auth_group table
//class CGroupTable : public CPowerTableBase
//{
//public:
//	CGroupTable();
//	~CGroupTable();
//
//	std::string GetAddSql();
//	std::string GetModifySql();
//	virtual void Init();
//public:
//	std::string name;
//};
//
////auth_user_groups table
//class CUserGroupTable : public CPowerTableBase
//{
//public:
//	CUserGroupTable();
//	~CUserGroupTable();
//
//	std::string GetAddSql();
//	std::string GetModifySql();
//	virtual void Init();
//public:
//	int user_id, group_id;
//};
////auth_group_permissions table
//class CGroupPermissionsTable : public CPowerTableBase
//{
//public:
//	CGroupPermissionsTable();
//	~CGroupPermissionsTable();
//
//	std::string GetAddSql();
//	std::string GetModifySql();
//	virtual void Init();
//public:
//	int group_id, permission_id;
//};
//
////auth_user_user_permissions table
//class CUserUserPermissionsTable : public CPowerTableBase
//{
//public:
//	CUserUserPermissionsTable();
//	~CUserUserPermissionsTable();
//
//	std::string GetAddSql();
//	std::string GetModifySql();
//	virtual void Init();
//public:
//	int user_id, permission_id;
//};
//
////django_content_type table
//class CContentTypeTable : public CPowerTableBase
//{
//public:
//	CContentTypeTable();
//	~CContentTypeTable();
//
//	std::string GetAddSql();
//	std::string GetModifySql();
//	virtual void Init();
//public:
//	std::string app_label, model;
//};
//
//class CDMUserManager
//{
//public:
//	CDMUserManager();
//	~CDMUserManager();
//
//	CUserTable m_userInfo;
//	std::vector<CPermissionTable> m_vecPermission;
//	//std::vector<CGroupTable> m_vecGroup;
//
//
//};