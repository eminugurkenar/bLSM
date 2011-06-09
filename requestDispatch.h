/*
 * requestDispatch.h
 *
 *  Created on: Aug 11, 2010
 *      Author: sears
 */

#ifndef REQUESTDISPATCH_H_
#define REQUESTDISPATCH_H_
#include "network.h"
#include "datatuple.h"
#include "logstore.h"
template<class HANDLE>
class requestDispatch {
private:
  static inline int op_insert(logtable * ltable, HANDLE fd, datatuple * tuple);
  static inline int op_test_and_set(logtable * ltable, HANDLE fd, datatuple * tuple, datatuple * tuple2);
  static inline int op_find(logtable * ltable, HANDLE fd, datatuple * tuple);
  static inline int op_scan(logtable * ltable, HANDLE fd, datatuple * tuple, datatuple * tuple2, size_t limit);
  static inline int op_bulk_insert(logtable * ltable, HANDLE fd);
  static inline int op_flush(logtable * ltable, HANDLE fd);
  static inline int op_shutdown(logtable * ltable, HANDLE fd);
  static inline int op_stat_space_usage(logtable * ltable, HANDLE fd);
  static inline int op_stat_perf_report(logtable * ltable, HANDLE fd);
  static inline int op_stat_histogram(logtable * ltable, HANDLE fd, size_t limit);
  static inline int op_dbg_blockmap(logtable * ltable, HANDLE fd);
  static inline int op_dbg_drop_database(logtable * ltable, HANDLE fd);
  static inline int op_dbg_noop(logtable * ltable, HANDLE fd);
  static inline int op_dbg_set_log_mode(logtable * ltable, HANDLE fd, datatuple * tuple);

public:
  static int dispatch_request(HANDLE f, logtable * ltable);
  static int dispatch_request(network_op_t opcode, datatuple * tuple, datatuple * tuple2, logtable * ltable, HANDLE fd);
};
#endif /* REQUESTDISPATCH_H_ */
