/* php_auto_instr_cfg extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "php_php_auto_instr_cfg.h"
#include "php_auto_instr_cfg_arginfo.h"

/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
	ZEND_PARSE_PARAMETERS_START(0, 0) \
	ZEND_PARSE_PARAMETERS_END()
#endif

/* {{{ void test1() */
PHP_FUNCTION(test1)
{
	ZEND_PARSE_PARAMETERS_NONE();

	php_printf("The extension %s is loaded and working!\r\n", "php_auto_instr_cfg");
}
/* }}} */

/* {{{ string test2( [ string $var ] ) */
PHP_FUNCTION(test2)
{
	char *var = "World";
	size_t var_len = sizeof("World") - 1;
	zend_string *retval;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_STRING(var, var_len)
	ZEND_PARSE_PARAMETERS_END();

	retval = strpprintf(0, "Hello %s", var);

	RETURN_STR(retval);
}
/* }}}*/

/* {{{ PHP_RINIT_FUNCTION */
PHP_RINIT_FUNCTION(php_auto_instr_cfg)
{
#if defined(ZTS) && defined(COMPILE_DL_PHP_AUTO_INSTR_CFG)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION(php_auto_instr_cfg)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "php_auto_instr_cfg support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ php_auto_instr_cfg_module_entry */
zend_module_entry php_auto_instr_cfg_module_entry = {
	STANDARD_MODULE_HEADER,
	"php_auto_instr_cfg",					/* Extension name */
	ext_functions,					/* zend_function_entry */
	NULL,							/* PHP_MINIT - Module initialization */
	NULL,							/* PHP_MSHUTDOWN - Module shutdown */
	PHP_RINIT(php_auto_instr_cfg),			/* PHP_RINIT - Request initialization */
	NULL,							/* PHP_RSHUTDOWN - Request shutdown */
	PHP_MINFO(php_auto_instr_cfg),			/* PHP_MINFO - Module info */
	PHP_PHP_AUTO_INSTR_CFG_VERSION,		/* Version */
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_PHP_AUTO_INSTR_CFG
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(php_auto_instr_cfg)
#endif
