<?php
	/*
	 * Forumsys
	 * By Jared Yanovich <jaredy@closeedge.net>
	 * Monday, March 17, 2003 10:52:22 AM
	 *
	 * extra.inc - extra/external routines
	 */

	if (!function_exists("array_strip_slashes"))
	{
		/*
		 * This function removes the slashes
		 * on escaped data from all values in
		 * an array.
		 */
		function array_strip_slashes(&$data)
		{
			if (!is_array($data))
				return;

			foreach ($data as $key => $val)
			{
				if (is_array($val))
					array_strip_slashes($data[$key]);
				else
					$data[$key] = stripSlashes($val);
			}

			return;
		}
	}

	if (!function_exists("mail_is_valid"))
	{
		function mail_is_valid($email)
		{
			$matches = array();

			# Make sure the '@' exists...
			if (preg_match("/^.+@(.+)$/",$email,$matches))
			{
				# As well as a valid hostname
/*
 * Note: There is a potential bug here. "Mis-configured"
 * hosts can allow mail servers for domains on which you
 * cannot perform NS lookups, since A and MX records
 * differ. Since getmxrr() does not exist on Windows,
 * until the equivalent exists, that platform will suffer.
 */
				if (forumsys_conf("check_email_addr"))
				{
					if (PHP_OS == "Windows")
						return getHostByName($matches[1]) != $matches[1];
					else
						return getmxrr($matches[1],$addr);
				} else {
					# FALSE and 0 are different in !== compared to !=
					return strpos($matches[1],".") !== FALSE;
				}
			} else {
				return FALSE;
			}
		}
	}

	if (!function_exists("unHTMLEntities"))
	{
		# Shamelessly taken from PHP.net
		function unHTMLEntities($str)
		{
			return	strtr
				(
					$str,
					array_flip
					(
						get_html_translation_table(HTML_ENTITIES)
					)
				);
		}
	}

	if (!function_exists('file_dump'))
	{
		function file_dump($file,$data)
		{
			global $php_errormsg;

			$fp = @fopen($file,"w");

			if (!$fp)
				forumsys_handle_error("file_dump(): unable to write file; file: $file");

			fputs($fp,$data);

			fclose($fp);

			return $data;
		}
	}

	if (!function_exists('file_get_contents'))
	{
		function file_get_contents($file)
		{
			global $php_errormsg;

			$fp = @fopen($file,"r");

			if (!$fp)
				forumsys_handle_error("file_get_contents(): unable to read file; file: $file");

			$size = @filesize($file);
			$data = @fread($fp,$size);

			fclose($fp);

			return $data;
		}
	}
?>
