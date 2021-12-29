#include "sql_parser_yd.h"
#include "yd_spinlock.h"
#include "../sql/SQLStatement.h"
#include <sys/time.h>

using namespace std;
using namespace hsql;
yd_spinlock_t yd_spinlock;

table_records  table_DML_modify;

static int add_table_record(string tab_name, uint64_t tv_sec)
{
  yd_spin_lock(yd_spinlock);
  table_records::iterator iter = table_DML_modify.find(tab_name);
  if (iter != table_DML_modify.end())
  {
    if (iter->second >= tv_sec)
    {
      yd_spin_unlock(yd_spinlock);
      return 0;
    }
  }
  table_DML_modify[tab_name] = tv_sec;
  yd_spin_unlock(yd_spinlock);
  return 0;
}

static uint64_t get_current_timestamp()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec  + tv.tv_usec / 1000000.0;
}

uint64_t get_table_modify_time(string tab_name)
{
  yd_spin_lock(yd_spinlock);
  table_records::iterator iter = table_DML_modify.find(tab_name);
  if (iter == table_DML_modify.end())
  {
    yd_spin_unlock(yd_spinlock);
    return 0;
  }
  yd_spin_unlock(yd_spinlock);
  return iter->second;
}

void add_table_records_low(const SQLStatement* stmt) 
{
  uint64_t nowint = get_current_timestamp();
  switch(stmt->type())
  {
  case kStmtInsert:
  case kStmtDelete:
    add_table_record(((const InsertStatement*) stmt)->tableName, nowint);
    break;
  case kStmtUpdate:
    add_table_record(((const UpdateStatement*) stmt)->table->name, nowint);
      break;
    /* 
 case kStmtDrop:
    remove_table_drop((const DropStatement*) stmt, nowint.tv_sec);
    break;
  case kStmtTruncate:
    add_table_truncate((const TruncateStatement*) stmt, nowint.tv_sec);
    break;
  case kStmtAlter:
    add_table_alter((const AlterStatement*) stmt, nowint.tv_sec);
    break;
    */
  default:
    break;
  }
}

int add_table_records(const char *query)
{
    // parse a given query
  hsql::SQLParserResult result;
  hsql::SQLParser::parse(string(query), &result);
  if (result.isValid()) 
  {
    for (auto i = 0u; i < result.size(); ++i) 
    {
      const SQLStatement* stmt = result.getStatement(i);
      if (stmt->type() == kStmtInsert || stmt->type() == kStmtUpdate ||  stmt->type() == kStmtDelete)
        add_table_records_low(stmt);
    }
    return 0;
  } 
  else 
  {
    return -1;
  }
}

void  get_table_name(TableRef* table, vector<string> &tables);

void get_table_name_from_select_stmt(const SelectStatement* stmt, vector<string>&tables)
{
  if (stmt->fromTable != nullptr) {
    get_table_name(stmt->fromTable, tables);
  }
  
  if (stmt->setOperations != nullptr) {
    for (SetOperation* setOperation : *stmt->setOperations) {
      get_table_name_from_select_stmt(setOperation->nestedSelectStatement, tables);
    }
  }
}

void  get_table_name(TableRef* table, vector<string> &tables)
{
    switch (table->type) {
    case kTableName:
      tables.push_back(std::string(table->name));
      break;
    case kTableSelect:
      get_table_name_from_select_stmt(table->select, tables);
      break;
    case kTableJoin:
      get_table_name(table->join->left, tables);
      get_table_name(table->join->right, tables);
      break;
    case kTableCrossProduct:
      for (TableRef* tbl : *table->list)
        get_table_name(tbl, tables);
      break;
    default:
      break;
    }
}

int get_table_names(const char *query, vector<string> &tables)
{
      // parse a given query
  hsql::SQLParserResult result;
  hsql::SQLParser::parse(string(query), &result);
  if (result.isValid()) 
  {
    for (auto i = 0u; i < result.size(); ++i) 
    {
      const SQLStatement* stmt = result.getStatement(i);
      if (stmt->type() == kStmtSelect)
        get_table_name_from_select_stmt((const SelectStatement*)stmt, tables);
    }
    return 0;
  } 
  else 
  {
    return -1;
  }
}

int refresh_destnation_hostgroup_for_replication_lag(const char *query, int default_dst_hg)
{
  vector<std::string> table_names ={};
  int ret = get_table_names(query, table_names);
  if (ret < 0)
    return default_dst_hg;
  uint64_t now = get_current_timestamp();
  bool tb_found = false;
  for (string tb_name: table_names)
  {
    table_records::iterator iter =  table_DML_modify.find(tb_name);
    if (iter != table_DML_modify.end())
    {
      if (now - iter->second  > MAX_REPLATION_LAG_TIME)
        return default_dst_hg;
      tb_found  = true;
    }
  }
  if (tb_found == true)
 {
    return DEFAULT_READ_HOSTGROUP;
  }
  else{
    return default_dst_hg;
  }   
}

int  show_table_modify_records()
{
  yd_spin_lock(yd_spinlock);
  table_records::iterator iter = table_DML_modify.begin();
  if (iter != table_DML_modify.end())
  {
    cout << "tab_name: " << iter->first << ", modify time: " << iter->second << endl;
  }
  yd_spin_unlock(yd_spinlock);
  return 0;
}
