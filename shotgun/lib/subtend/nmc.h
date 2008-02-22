/* nmc == native method context */

#include <setjmp.h>

#include "shotgun/lib/cpu.h"
#include "shotgun/lib/subtend/nmethod.h"

struct rni_nmc {
  int num_handles;
  int used;
  rni_handle **handles;
  native_method *method;
  rni_handle *value;
  int args;
  OBJECT symbol;
  int system_set;
  int cont_set;
  int jump_val;
  ucontext_t system;
  /* work around a bug in 10.5's libc versus header files */
#if __DARWIN_UNIX03
  _STRUCT_MCONTEXT __system_mc;
#endif
  ucontext_t cont;
#if __DARWIN_UNIX03
  _STRUCT_MCONTEXT __cont_mc;
#endif
  
  int setup_context;
  
  void *stack;
  int stack_size;
  
  void *local_data;
};

typedef struct rni_nmc rni_nmc;

struct rni_context {
  STATE;
  cpu cpu;
  rni_nmc *nmc;
  struct fast_context *fc;
  void *fault_address;
};

typedef struct rni_context rni_context;

void subtend_set_context(STATE, cpu c, rni_nmc *nmc);
rni_context* subtend_retrieve_context();
rni_nmc *nmc_new_standalone();
void nmc_activate(STATE, cpu c, OBJECT nmc, OBJECT val, int reraise);
OBJECT nmc_new(STATE, OBJECT nmethod, OBJECT sender, OBJECT recv, OBJECT name, OBJECT block, int args);
void _nmc_save_stack(rni_nmc *nmc, unsigned long *bottom, unsigned long *top);
rni_handle *nmc_handle_new(rni_nmc *n, rni_handle_table *tbl, OBJECT obj);

#define CALL_METHOD 1
#define CLEANUP 2
#define RAISED_EXCEPTION 3
#define ALL_DONE 4
#define CALL_ERROR 5
#define SEGFAULT_DETECTED 6

