<?php
	/*
	 * Forumsys
	 * By Jared Yanovich <jaredy@closeedge.net>
	 * Monday, March 17, 2003 11:44:35 AM
	 *
	 * misc.inc - miscellaneous routines
	 */

	# forumsys_str_parse() types
	define("FORUMSYS_STR_NONE",	0,TRUE);
	define("FORUMSYS_STR_URL",	1,TRUE);
	define("FORUMSYS_STR_HTML",	2,TRUE);
	define("FORUMSYS_STR_ALL",	FORUMSYS_STR_URL | FORUMSYS_STR_HTML,TRUE);

	# forumsys_build_path() types
	define("FORUMSYS_PATH_ABS",	1,TRUE);
	define("FORUMSYS_PATH_REL",	2,TRUE);
	define("FORUMSYS_PATH_SYS",	3,TRUE);

	function forumsys_build_path($type = FORUMSYS_PATH_REL)
	{
		$path = "";

		switch ($type)
		{
			case FORUMSYS_PATH_REL: $path = forumsys_conf("web_root") . forumsys_conf("path"); break;
			case FORUMSYS_PATH_SYS: $path = forumsys_conf("sys_root") . forumsys_conf("path"); break;
			case FORUMSYS_PATH_ABS: $path = forumsys_conf("site_uri") . forumsys_conf("path"); break;
			default:
				forumsys_handle_error("forumsys_build_path(): invalid type; type: $type");
		}

		return $path;
	}

	function forumsys_gen_class()
	{
		static $count = 1;

		return "forumsysData" . (($count++ % 2) + 1);
	}

	function forumsys_move_file($src,$dst)
	{
		global $php_errormsg;

		if (!@copy($src,$dst))
			forumsys_handle_error("forumsys_move_file(): cannot copy file; src: $src; dst: $dst");

		forumsys_unlink($src);

		return;
	}

	function forumsys_get_dbh()
	{
		static $dbh = array();

		$dbh_type = forumsys_conf("dbh_type");

		if (!array_key_exists($dbh_type,$dbh))
		{
			$class	= "DBH_$dbh_type";
			$file	= "DBH-$dbh_type/1.7.inc";

			require_once $file;

			$dbh[$dbh_type] =	new $class
						(
							forumsys_conf("dbh_host"),
							forumsys_conf("dbh_username"),
							forumsys_conf("dbh_password"),
							forumsys_conf("dbh_database")
						);
		}

		return $dbh[$dbh_type];
	}

	function forumsys_get_of()
	{
		static $of = array();

		$of_type = forumsys_conf("of_type");

		if (!array_key_exists($of_type,$of))
		{
			$class		= "OF_$of_type";
			$file		= "OF-$of_type/1.7.inc";

			require_once $file;

			$of[$of_type]	= new $class();
		}

		return $of[$of_type];
	}

	function forumsys_handle_error($msg)
	{
		static $count = 0; # To monitor infinite recursion
		global $php_errormsg;

		$msg .=	"; Date: " . date("F d, Y h:i:s A") .
			"; File: " . $_SERVER["PHP_SELF"];

		if (ini_get('track_errors'))
			$msg .= "; PHP Error: $php_errormsg";

		$msg .= "\n";

		if (forumsys_conf("trigger_errors"))
			trigger_error($msg,E_USER_ERROR);

		if ($log = forumsys_conf("error_log"))
			if ($fp = @fopen($log,"a"))
			{
				fputs($fp,$msg);
				fclose($fp);
			}

		# Mail error report
		if (forumsys_conf("mail_errors"))
			@mail # forumsys_mail() might recursively throw errors
			(
				forumsys_conf("admin_email"),
				forumsys_conf("site_name") . " Forumsys Error",
				"A Forumsys error has occured:\n\n$msg",
				"From: " . forumsys_conf("admin_email")
			);

		if (forumsys_conf("show_errors"))
		{
			if ($count++)
			{
				# Ok, no fancy stuff, just exit
				echo	"The following error occurred while processing that directive: $msg";
			} else {
				$of = forumsys_get_of();

				echo	forumsys_get_template("header"),
					$of->header("Fatal Error"),
					$of->p("The following error occurred while processing that directive: $msg"),
					forumsys_get_template("footer");
			}
		} else {
			if ($count++)
			{
				# Same here; nothing fancy
				echo	"An error has occurred. Please contact the site administrator and report this error.";
			} else {
				$of = forumsys_get_of();

				echo	forumsys_get_template("header"),
					$of->header("Fatal Error"),
					$of->p("An error has occurred. Please contact the site administrator and report this error."),
					forumsys_get_template("footer");
			}
		}

		exit(1);
	}

	function forumsys_redirect($url)
	{
		# Clear buffer if there's stuff in it
		if (headers_sent())
			ob_end_clean();
/*
 * NOTE: $url here *should* be an absolute URI. Perhaps if it doesn't
 * contain an 'proto://...' we should give it one?
 */

		if (preg_match("!^[a-z]+:!",$url))
		{
			# Checks for a complete URI; we're done

		} elseif (preg_match("!^/!",$url)) {

			# Checks for relative path
			$url    = forumsys_conf("site_uri") . $url;
		} else {
			# Must be relative to '.'
			$path	= $_SERVER["PHP_SELF"];
			$path	= preg_replace("![^/]+$!","",$path);
			$url	= forumsys_conf("site_uri") . $path . $url;
		}

		header("Location: $url");

		exit(0);
	}

	function forumsys_mail($to,$from,$subject,$body)
	{
		global $php_errormsg;

		# Strip possible CRLFs in the header
		$from = preg_replace("/(?:\r\n|[\r\n])+/","",$from);

		if (!@mail($to,$subject,$body,"From: $from"))
			forumsys_handle_error("forumsys_mail(): error in mail()");

		return;
	}

	/*
	 * This temporary function is just used to clean up attributes
	 * found by forumsys_str_parse()
	 */
	function forumsys_clean_attr($attr)
	{
		static $story_strip_expr = NULL;

		# Attributes will be matched with their delimiters (" or ')
		$attr = preg_replace("/^&quot;|^'|&quot;$|'$/","",$attr);

		# Attributes will have been subject to htmlEntities()
		$attr = unHTMLEntities($attr);

		if ($story_strip_expr === NULL)
			$story_strip_expr = forumsys_conf("story_strip_expr");


		return $attr;
	}

	function forumsys_css_remove($ids,$data)
	{
		$props	= explode(";",$data);
		$len	= count($props);

		for ($i = 0; $i < $len; $i++)
		{
			list ($name) = explode(":",$props[$i]);

			# Found it, remove property
			if (in_array($name,$ids))
				$props[$i--] = $props[--$len];
		}

		return join(";",$props);
	}

	/*
	 * This temporary function is just used to check potentially
	 * arbitrarily-protocoled URIs for disallowed protocols
	 */
	function forumsys_check_proto($url)
	{
		if (preg_match("/^\s*(\w+):/",$url,$matches))
			return in_array($matches[1],forumsys_conf("story_allowed_proto"));

		return TRUE;
	}

	function forumsys_str_parse($str,$flags = FORUMSYS_STR_ALL)
	{
		# Convert HTML
		$str = htmlEntities($str);

		if ($flags & FORUMSYS_STR_HTML)
		{
			/*
			 * Note: should probably be an option including leaving
			 * alone, blocking, and truncating.
			 */

			# Allowed HTML tags
			$html	= strToLower(join("|",forumsys_conf("story_allowed_html")));
			$str	=	preg_replace
					(
						"!
							&lt;			# HTML is escaped
							(
								/?		# Start or end
								(?i:$html)	# Case-insensitive tag match
								\b		# Word boundary
								.*?		# Rest of tag
							)
							&gt;
						!x",
						"<$1>",# "<$tag parse_attr($attr)>" ?
						$str
					);

			# Allowed HTML attributes
			$story_allowed_attr	= forumsys_conf("story_allowed_attr");
			$story_attr_proto	= forumsys_conf("story_attr_proto");

			while (1)
			{
				$new = preg_replace
				(
					"!
					( # HTML tag ($1)
						<				# HTML tag (allowed & matched)
							\w+
							(?:\s*\w+=\".*?\")*	# Previously-allowed attributes
							\s?			# Just for formatting purposes
					)
							\s*
							(\w+)			# Attribute name ($2)
							\s*
							=
							\s*
							( # Attribute ($3) (One of the following...)
								&quot;.*?&quot;	# Double quotes
								|
								'.*?'		# Single quotes (not changed
								|		#	in htmlEntities()
								[^\"\s>]+	# No quotes, bounded by whitespace
							)
					( # End of tag ($4)
							.*?			# Other tag content
						>				# End of tag
					)
					!xe",
					'
						"$1" .	# Tag and previous attributes
						(
							/*
							 * Validate attribute, must be allowed in
							 * story_allowed_attr and if its of type to be
							 * checked for malicious protocols, subject
							 * to checking
							 */
							in_array("$2",$story_allowed_attr) &&
							(
								in_array("$2",$story_attr_proto) ?

								# Subject to checking if special attribute
								forumsys_check_proto(forumsys_clean_attr("$3"))

								# Else it is good
								: TRUE
							) ?

							# Formatted attribute
							" $2=\"" . forumsys_clean_attr("$3") . "\""

							# Or nothing
							: ""
						) .
						"$4"
					',
					$str,
					1 # We can only perform this match once per loop
				);

				if ($new == $str)
					# Nothing changed; we're done
					break;
				else
					# "Save changes" and continue to next attribute
					$str = $new;
			}
		}

		# Turn URLs into hyperlinks
		if ($flags & FORUMSYS_STR_URL)
		{
			if (forumsys_conf("story_auto_urls"))
			{
				$tlds = "(?:" . join("|",forumsys_conf("story_auto_url_tlds")) . ")";

				/*
				 * Since negative lookbehind assertions are not
				 * supported, we'll save want we don't want to
				 * match and check its absence in our
				 * replacement section.
				 *
				 * Below is the following:
				 *	\\1 -> what was matched
				 *	\\2 -> what we don't want to match
				 *	\\3 -> the URL
				 */
/*
 * NOTE: we have a potential bug as this rewrites URLs assuming they're in
 * HTML format. It should actually grab the output of $ns_of->link() and make
 * changes relative to that accordingly.
 */
				$str	=	preg_replace
						(
							"{(	# Everything - in case of failed match ($1)
#								(?<!<a\b[^>]+\bhref\s*=\s*[\"']?|<a\b[^>]+\bhref\b[^>]+>)
								# What we don't want to match ($2)
								(
									# <a href=URL
									<a\n[^>]+\bhref\s*=\s*[\"']?
									|
									# <a>URL</a>
									<a[^>]*>(?!.*</a>.*)
									|
									# URL
								)
								# The URL ($3)
								(
									(?:http:/{1,3})?
									(?:www\\.)?
								#	(?!\d+\\.\d+\b) # To prevent, e.g., `3.3'
									[a-zA-Z0-9-]
									[a-zA-Z0-9.-]+
									\\.
									(?:
										" .
										# check if they want us to match preset tlds
										($tlds == "(?:)" ? "
											[a-zA-Z]+ # tld
										" : "
											$tlds\b
								#			(?=\s|/|$)
										") .
										"
										(?:
											/
											\S*?
											(?=
												[,.:;!]+$|
												[,.:;!]+\s|
												$|
												\s
											) # No trailing punctuation
										)?
								#		\S*
									)
								)
							)}iex",
							'
								"$2" ? "$1" :
								"<a href=\\"" .
								(
									preg_match("!^http://!","$3") ?
									"" :
									"http://"
								) .
								(
									preg_match
									(
										"!/!",
										preg_replace("!^http://!","","$3")
									)
									?
									"$3"
									:
									"$3/"
								) .
								"\\">$3</a>"
							',
							$str
						);
			}
		}

		# Fix newline madness
		$str = preg_replace("/\r\n|(?!<\r)\n|(?!<\n)\r/","<br />",$str);

		# Break up long words
		$str = preg_replace("!([^\s<>/\"']{" . forumsys_conf("story_word_length") . "})!","\\1 ",$str);

		return $str;
	}

	function forumsys_unlink($file)
	{
		global $php_errormsg;

		if (!@unlink($file))
			forumsys_handle_error("forumsys_unlink(): cannot unlink() file; file: $file");

		return;
	}
?>
