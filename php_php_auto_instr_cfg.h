/* php_auto_instr_cfg extension for PHP */

#ifndef PHP_PHP_AUTO_INSTR_CFG_H
# define PHP_PHP_AUTO_INSTR_CFG_H

extern zend_module_entry php_auto_instr_cfg_module_entry;
# define phpext_php_auto_instr_cfg_ptr &php_auto_instr_cfg_module_entry

# define PHP_PHP_AUTO_INSTR_CFG_VERSION "0.1.0"

# if defined(ZTS) && defined(COMPILE_DL_PHP_AUTO_INSTR_CFG)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#endif	/* PHP_PHP_AUTO_INSTR_CFG_H */
