<?php
	/*
	 * Forumsys
	 * By Jared Yanovich <jaredy@closeedge.net>
	 * Monday, March 17, 2003 10:35:07 AM
	 *
	 * main.inc - main routines/declarations
	 */

	define("FORUMSYS_VERSION",	"-current",	TRUE);	# Version information
	define("FORUMSYS_BUF_SIZE",	1024,		TRUE);	# Buffer size
	define("E_FORUMSYS_NONE",	0,		TRUE);	# Error codes

	function forumsys_conf()
	{
		static $vars	= array();

		$args		= func_get_args();
		$argc		= func_num_args();

		if ($argc == 2)
		{
			$vars[$args[0]] = $args[1];

		} elseif ($argc != 1) {

			forumsys_handle_error("forumsys_conf(): bad arguments");
		}

		if (!array_key_exists($args[0],$vars))
			forumsys_handle_error("forumsys_conf(): configuration value requested before supplied; key: $args[0]");

		return $vars[$args[0]];
	}

	require_once 'CPL/1.0.inc';
	require_once 'forumsys-config.inc';
	require_once 'TimeStamp/1.5.inc';

	$forumsys_sys_root = forumsys_conf('sys_root');

	require_once "$forumsys_sys_root/board.inc";
	require_once "$forumsys_sys_root/cache.inc";
	require_once "$forumsys_sys_root/cat.inc";
	require_once "$forumsys_sys_root/crypt.inc";
	require_once "$forumsys_sys_root/extra.inc";
	require_once "$forumsys_sys_root/isr.inc";
	require_once "$forumsys_sys_root/misc.inc";
	require_once "$forumsys_sys_root/of.inc";
	require_once "$forumsys_sys_root/post.inc";
	require_once "$forumsys_sys_root/reply.inc";
	require_once "$forumsys_sys_root/session.inc";
	require_once "$forumsys_sys_root/user.inc";
	require_once "$forumsys_sys_root/userfields.inc";
	require_once "$forumsys_sys_root/xml.inc";

	if (ini_get('magic_quotes_gpc'))
	{
		array_strip_slashes($_POST);
		array_strip_slashes($_GET);
		array_strip_slashes($_REQUEST);
		array_strip_slashes($_COOKIE);
		array_strip_slashes($_SESSION);
	}

	ini_set("magic_quotes_runtime",FALSE);
?>
