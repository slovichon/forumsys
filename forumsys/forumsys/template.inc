<?php
	/*
	 * Forumsys
	 * By Jared Yanovich <jaredy@closeedge.net>
	 * Thursday, March 27, 2003 08:35:02 PM
	 *
	 * template.inc - template routines
	 */

	function forumsys_get_template()
	{
		static $_use_cache;
		global $php_errormsg;

		if ($_use_cache == NULL)
			$_use_cache = forumsys_conf("use_cache");

		$output		= "";
		$args		= func_get_args();
		$_type		= $args[0];
		$_id		= @join('-',$args);
		$cache_dir	= forumsys_conf("cache_dir");
		$templates_dir	= forumsys_conf("templates_dir");

		$NO_CACHE	= FALSE;

		# If something has changed, rebuild it
		if ($_use_cache && forumsys_cache_is_fresh($_id))
		{
			# Nothing has changed, used cached version
			$output = file_slurp("$cache_dir/cache-$_id");
		} else {
			# Load template-specific dependencies
			$ret	= include($templates_dir . "/$_type.inc");

			# Load structure and check for errors
			if (!$ret)
				return NULL;

			# Load template, substitute variables, save output
			ob_start();
			include("$templates_dir/$_type.t");
			$output	= ob_get_contents();
			ob_end_clean();

			# Substitute variables
			$output =	preg_replace
					(
						"/
							{?				# Optional { ... } delimiter
							(
								\\\$			# Dollar
								[a-zA-Z_]		# First char
								[a-zA-Z_0-9]+		# Subsequent chars
								(?:
									\[		# Subscript
									([\"'])		# Quotes
									.*?		# Index
									\\2		# Matching quote
									\]
								)*
							)
							}?
						/xe",
						"$1", # will cause a problem with $arr[\"index\"]
						$output
					);

			# Write over old cached version
			if (!$NO_CACHE)
				file_dump("$cache_dir/cache-$_id",$output);
		}

		return $output;
	}
?>
