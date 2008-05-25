#include "shotgun/lib/shotgun.h"
#include "shotgun/lib/string.h"

/* Copied from ruby.h to avoid including it here. */
struct RString {
  char *ptr;
  int len;
};

typedef struct RString RString;

rni_handle_table *handle_table_new() {
  rni_handle_table *tbl;
  
  tbl = ALLOC(rni_handle_table);
  tbl->next = 0;
  tbl->id = 1;
  tbl->total = 100;
  tbl->entries = ALLOC_N(rni_ht_entry*, tbl->total);
  return tbl;
}

rni_handle *handle_allocate() {
  rni_handle *h;
  
  h = ALLOC(rni_handle);
  h->flags = 0;
  h->handle_id = 0;
  h->table_idx = 0;
  h->data = 0;
  
  return h;
}

rni_handle *handle_new(rni_handle_table *tbl, OBJECT obj) {
  rni_handle *h;
  rni_ht_entry *e;
    
  /* Create the handle. By default, handles are local only. */
  h = handle_allocate();
  h->handle_id = tbl->id++;
  h->table_idx = tbl->next;
  
  /* Create the table entry. */
  e = ALLOC(rni_ht_entry);
  e->handle_id = h->handle_id;
  e->object = obj;
  e->rh = h;
  
  /* Put the entry in the table. */
  tbl->entries[h->table_idx] = e;
  
  /* Move next forward to the next empty slot. */
  while(tbl->next < tbl->total && tbl->entries[tbl->next]) {
    tbl->next++;
  }
  
  /* Resize the entries array if it's not big enough. */
  if(tbl->next == tbl->total) {
    int s = tbl->total;
    tbl->total = tbl->total + 25;
    tbl->entries = realloc(tbl->entries, tbl->total * sizeof(rni_ht_entry*));
    for(;s < tbl->total; s++) {
      tbl->entries[s] = NULL;
    }
  }
  
  return h;
}

/*

void handle_become_string(rni_handle *h, char *str, int len) {
  bstring bstr;
  SET_FLAG(h, STRING_FLAG);
  bstr = blk2bstr(str, len);
  h->data = (void*)bstr;
}

OBJECT _handle_convert_string(STATE, rni_handle_table *tbl, rni_handle *h) {
  OBJECT obj;
  
  obj = string_new2(state, AS_STR(h)->str, AS_STR(h)->len);
  _handle_table_assign(tbl, h, obj);
  return obj;
}

void handle_become_array(rni_handle *h, int len) {
  ptr_array gary;
  SET_FLAG(h, ARRAY_FLAG);
  gary = ptr_array_new(len);
  h->data = (void*)gary;
}

OBJECT _handle_convert_array(STATE, rni_handle_table *tbl, rni_handle *h) {
  OBJECT obj;
  int i;
  
  obj = array_new(state, AS_ARY(h)->len);
  for(i = 0; i < AS_ARY(h)->len; i++) {
    array_set(state, obj, i, handle_to_object(state, tbl, AS_ARY(h)->pdata[i]));
  }
  _handle_table_assign(tbl, h, obj);
  return obj;
}

rni_handle *handle_detached_string(rni_handle_table *tbl, char *str, int len) {
  rni_handle *h;
  h = handle_allocate();
  handle_become_string(h, str, len);
  return h;
}

rni_handle *handle_detached_array(rni_handle_table *tbl, int len) {
  rni_handle *h;
  h = handle_allocate();
  handle_become_array(h, len);
  return h;
}

*/

/* Check if the handle has RStruct data in it. 
   If it does we have to copy RStruct data back to the object. */
void check_rstruct_data(STATE, rni_handle *h, OBJECT o) {
  if(!h->data) {
    return;
  }
  if (STRING_P(o)) {
    RString *rs = (RString *)h->data;
    string_cstr_overwrite(state, o, rs->ptr, rs->len);
    XFREE(rs);
    h->data = 0;
  } else if (ARRAY_P(o)) {
    //TODO: overwrite array in object
  }
}

/* Check all created handles looking for RStruct data on them. */
void check_rstruct_data_in_handles(STATE, rni_handle_table *tbl) {
  rni_ht_entry *e;
  rni_handle *h;
  int i;

  for(i = 0; i < tbl->total; ++i) {
    e = tbl->entries[i];
    if(!e) {
      continue;
    }
    h = e->rh;
    check_rstruct_data(state, h, e->object);
  }
}

OBJECT handle_to_object(STATE, rni_handle_table *tbl, rni_handle *h) {
  rni_ht_entry *e;
  
  if(!REFERENCE_P((OBJECT)h)) {
    return (OBJECT)h;
  }
  
  /* Oh! a detached handle! */
  if(!h->handle_id) {
    /*
    if(IS_FLAG(h, STRING_FLAG)) {
      return _handle_convert_string(state, tbl, h);
    } else if(IS_FLAG(h, ARRAY_FLAG)) {
      return _handle_convert_array(state, tbl, h);
    }
    */
    
    printf("Fuck. A detached handle with no data.\n");
    return Qnil;
  }
  
  e = tbl->entries[h->table_idx];
  
  /* Check that the handle and the table entry have the same handle_id.
     if they don't, someone is using this handle improperly. */

  if(e->handle_id != h->handle_id) {
    /* ERROR! */
    abort();
    return Qnil;
  }
  
  check_rstruct_data(state, h, e->object);

  return e->object;
}

void handle_make_global(rni_handle *h) {
  SET_FLAG(h, GLOBAL_FLAG);
}

void handle_clear_global(rni_handle *h) {
  CLEAR_FLAG(h, GLOBAL_FLAG);
}

int handle_is_global(rni_handle *h) {
  return IS_FLAG(h, GLOBAL_FLAG);
}

OBJECT handle_remove(rni_handle_table *tbl, rni_handle *h) {
  rni_ht_entry *e;
  e = tbl->entries[h->table_idx];
  
  if(e->handle_id != h->handle_id) {
    return Qnil;
  }
  
  tbl->entries[h->table_idx] = NULL;
  return e->object;
}

void handle_delete(rni_handle *h) {
  /*
  if(IS_FLAG(h, STRING_FLAG)) {
    bdestroy((bstring)h->data);
  } else if(IS_FLAG(h, ARRAY_FLAG)) {
    ptr_array_free(ptr_array)h->data);
  }
  */
  free(h);
}
