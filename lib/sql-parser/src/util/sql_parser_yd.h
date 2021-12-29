#include "../SQLParser.h"
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include<map>
#include <iostream>
using namespace std;
#define MAX_REPLATION_LAG_TIME (3)
#define DEFAULT_READ_HOSTGROUP (30)
typedef std::map<std::string, uint64_t> table_records;
vector<std::string> get_tables_names(const char *query);

int refresh_destnation_hostgroup_for_replication_lag(const char *query, int default_dst_hg);
int add_table_records(const char *query);
int show_table_modify_records();
