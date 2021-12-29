
#include <stdlib.h>
#include <string>

// include the sql parser
#include "SQLParser.h"

// contains printing utilities
#include "util/sqlhelper.h"
#include "util/sql_parser_yd.h"
#include "util/yd_spinlock.h"

extern yd_spinlock_t yd_spinlock;
int main(int argc, char* argv[]) {
  if (argc <= 1) {
    fprintf(stderr, "Usage: ./example \"SELECT * FROM test;\"\n");
    return -1;
  }
  std::string query = argv[1];
  yd_create_spinlock(&yd_spinlock);
  
  // check whether the parsing was successful

  add_table_records(query.c_str());
  show_table_modify_records();
  yd_destroy_spinlock(yd_spinlock);
}
