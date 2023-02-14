/* php_auto_instr_cfg extension for PHP */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "php_auto_instr_cfg_arginfo.h"
#include "php_php_auto_instr_cfg.h"

/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE()                                           \
  ZEND_PARSE_PARAMETERS_START(0, 0)                                            \
  ZEND_PARSE_PARAMETERS_END()
#endif

struct stack {
  const char *function_name;
  int size;
  struct stack *next;
};

void (*original_zend_execute_ex)(zend_execute_data *execute_data);
void (*original_zend_execute_internal)(zend_execute_data *execute_data,
                                       zval *return_value);
void new_execute_internal(zend_execute_data *execute_data, zval *return_value);
void execute_ex(zend_execute_data *execute_data);

#define DYNAMIC_MALLOC_SPRINTF(destString, sizeNeeded, fmt, ...)               \
  sizeNeeded = snprintf(NULL, 0, fmt, ##__VA_ARGS__) + 1;                      \
  destString = (char *)malloc(sizeNeeded);                                     \
  snprintf(destString, sizeNeeded, fmt, ##__VA_ARGS__)

FILE *fp;
HashTable function_set;
struct stack *head;

struct stack *make_node(const char *name, int s) {
  struct stack *node = (struct stack *)malloc(sizeof(struct stack));
  node->function_name = name;
  node->size = s;
  return node;
}

struct stack *push_on_stack(struct stack *current, const char *name) {
  int size = 0;
  if (current) {
    size = current->size + 1;
  }
  struct stack *head = make_node(name, size);
  head->next = current;
  return head;
}

struct stack *pop_from_stack(struct stack *current) {
  struct stack *head = current;
  if (current) {
    head = current->next;
    if (head)
      head->size = current->size - 1;
    free(current);
  }
  return head;
}

void traverse(struct stack *head) {
  struct stack *current = head;
  if (current && current->size >= 0) {
    fwrite("stack:\n", 6, 1, fp);
  }
  while (current) {
    fwrite("\t", sizeof(char), 1, fp);
    fwrite(current->function_name, 1, strlen(current->function_name), fp);
    fwrite("\n", sizeof(char), 1, fp);
    current = current->next;
  }
  if (head && head->size >= 0) {
    fwrite("\n", 1, 1, fp);
  }
}

const char *get_function_name(zend_execute_data *execute_data) {
  if (!execute_data->func) {
    return strdup("");
  }

  if (execute_data->func->common.scope &&
      execute_data->func->common.fn_flags & ZEND_ACC_STATIC) {
    int len = 0;
    char *ret = NULL;
    DYNAMIC_MALLOC_SPRINTF(ret, len, "%s::%s",
                           ZSTR_VAL(Z_CE(execute_data->This)->name),
                           ZSTR_VAL(execute_data->func->common.function_name));
    return ret;
  }

  if (Z_TYPE(execute_data->This) == IS_OBJECT) {
    int len = 0;
    char *ret = NULL;
    if (execute_data->func->common.function_name) {
      DYNAMIC_MALLOC_SPRINTF(
          ret, len, "%s->%s", ZSTR_VAL(execute_data->func->common.scope->name),
          ZSTR_VAL(execute_data->func->common.function_name));
    }
    return ret;
  }
  if (execute_data->func->common.function_name) {
    return strdup(ZSTR_VAL(execute_data->func->common.function_name));
  }
  return strdup("");
}

void register_execute_ex() {
  fp = fopen("functions.log", "w");

  zend_hash_init(&function_set, 0, NULL, NULL, 0);

  head = NULL;

  original_zend_execute_internal = zend_execute_internal;
  zend_execute_internal = new_execute_internal;

  original_zend_execute_ex = zend_execute_ex;
  zend_execute_ex = execute_ex;
}

void unregister_execute_ex() {

  zend_execute_internal = original_zend_execute_internal;
  zend_execute_ex = original_zend_execute_ex;

  zend_hash_destroy(&function_set);

  fclose(fp);
}

void update_function_set(const char *function_name) {
  if (function_name && strlen(function_name) != 0) {
    zval *val;
    zend_string *function_name_zend =
        zend_string_init(function_name, strlen(function_name), 0);
    if (!zend_hash_exists(&function_set, function_name_zend)) {
      zval tmp;
      ZVAL_STR(&tmp, function_name_zend);
      /* Add the wrapped zend_string to the HashTable */
      zend_hash_add(&function_set, function_name_zend, &tmp);
    }
    zend_string_release(function_name_zend);
  }
}

void execute_ex(zend_execute_data *execute_data) {
  const char *function_name;

  int argc;
  zval *argv = NULL;
  function_name = get_function_name(execute_data);
  update_function_set(function_name);
  if (function_name && strlen(function_name) > 0) {
    head = push_on_stack(head, function_name);
  }
  traverse(head);
  original_zend_execute_ex(execute_data);

  head = pop_from_stack(head);

  free((void *)function_name);
}

void new_execute_internal(zend_execute_data *execute_data, zval *return_value) {
  const char *function_name;

  int argc;
  zval *argv = NULL;
  function_name = get_function_name(execute_data);
  update_function_set(function_name);
  if (function_name && strlen(function_name) > 0) {
    head = push_on_stack(head, function_name);
  }
  traverse(head);
  if (original_zend_execute_internal) {
    original_zend_execute_internal(execute_data, return_value);
  } else {
    execute_internal(execute_data, return_value);
  }

  head = pop_from_stack(head);

  free((void *)function_name);
}

static PHP_MINIT_FUNCTION(php_auto_instr_cfg) {
  register_execute_ex();
  return SUCCESS;
}

static PHP_MSHUTDOWN_FUNCTION(php_auto_instr_cfg) {
  //   zend_ulong idx;
  //   zend_string *key;
  //   zval *val;

  //   char dump_function_set[20] = "dump_function_set\0";
  //   fwrite(dump_function_set, 1, strlen(dump_function_set), fp);
  //   fwrite("\n", sizeof(char), 1, fp);
  //   ZEND_HASH_FOREACH_KEY_VAL(&function_set, idx, key, val) {
  //     if (key) {
  // 		fwrite(ZSTR_VAL(key), 1, strlen(ZSTR_VAL(key)), fp);
  // 		fwrite("\n", sizeof(char), 1, fp);
  //     }
  //   } ZEND_HASH_FOREACH_END();
  //   unregister_execute_ex();
  return SUCCESS;
}

/* {{{ PHP_RINIT_FUNCTION */
PHP_RINIT_FUNCTION(php_auto_instr_cfg) {
#if defined(ZTS) && defined(COMPILE_DL_PHP_AUTO_INSTR_CFG)
  ZEND_TSRMLS_CACHE_UPDATE();
#endif

  return SUCCESS;
}
/* }}} */

PHP_RSHUTDOWN_FUNCTION(php_auto_instr_cfg) {
  //   zend_ulong idx;
  //   zend_string *key;
  //   zval *val;

  //   char dump_function_set[20] = "dump_function_set\0";
  //   fwrite(dump_function_set, 1, strlen(dump_function_set), fp);
  //   fwrite("\n", sizeof(char), 1, fp);
  //   ZEND_HASH_FOREACH_KEY_VAL(&function_set, idx, key, val) {
  //     if (key) {
  // 		fwrite(ZSTR_VAL(key), 1, strlen(ZSTR_VAL(key)), fp);
  // 		fwrite("\n", sizeof(char), 1, fp);
  //     }
  //   } ZEND_HASH_FOREACH_END();
  //   unregister_execute_ex();
  return SUCCESS;
}

/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION(php_auto_instr_cfg) {
  php_info_print_table_start();
  php_info_print_table_header(2, "php_auto_instr_cfg support", "enabled");
  php_info_print_table_end();
}
/* }}} */

/* {{{ php_auto_instr_cfg_module_entry */
zend_module_entry php_auto_instr_cfg_module_entry = {
    STANDARD_MODULE_HEADER,
    "php_auto_instr_cfg",              /* Extension name */
    ext_functions,                     /* zend_function_entry */
    PHP_MINIT(php_auto_instr_cfg),     /* PHP_MINIT - Module initialization */
    PHP_MSHUTDOWN(php_auto_instr_cfg), /* PHP_MSHUTDOWN - Module shutdown */
    PHP_RINIT(php_auto_instr_cfg),     /* PHP_RINIT - Request initialization */
    PHP_RSHUTDOWN(php_auto_instr_cfg), /* PHP_RSHUTDOWN - Request shutdown */

    PHP_MINFO(php_auto_instr_cfg),  /* PHP_MINFO - Module info */
    PHP_PHP_AUTO_INSTR_CFG_VERSION, /* Version */
    STANDARD_MODULE_PROPERTIES};
/* }}} */

#ifdef COMPILE_DL_PHP_AUTO_INSTR_CFG
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(php_auto_instr_cfg)
#endif
