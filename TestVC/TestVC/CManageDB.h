#pragma once

//#include "CConnectManagerBase.h"
#define  WIN32_LEAN_AND_MEAN
//#include <msgpack.hpp>
//#include <zmq.h>
#include "CppSQLite3.h"
#include <string>

class CManageDB /*: public CConnectManagerBase*/
{
public:
	CManageDB();
	~CManageDB();
	bool ConnectDB(const std::string &strDB);
	void CloseDB();
		
	static CManageDB *instance();
	static void ReleaseInstance();
	//bool ProcessCommand(const char * strCmd, size_t iSize);
	bool BeginTrans();
	bool CommitTrans();
	bool RollbackTrans();

public:
	CppSQLite3Query ExecQuery(const std::string &strSql);
	bool ExecCommand(const std::string &strSql);
	bool ExecCommandBin(const std::string &strSql, const unsigned char *pStr, size_t iSize);

	bool m_bActive;
	//msgpack::sbuffer sbuf;
private:
	CppSQLite3DB db;
	static CManageDB *s_pManager;

};

