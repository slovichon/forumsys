<?php
	# Version information
	define("FORUMSYS_VERSION","-current",TRUE);

	# Buffer size
	define("FORUMSYS_BUF_SIZE",1024,TRUE);

	# String parsing types
	define("FORUMSYS_STR_NONE",	0,TRUE);
	define("FORUMSYS_STR_URL",	1,TRUE);
	define("FORUMSYS_STR_HTML",	2,TRUE);
	define("FORUMSYS_STR_ALL",	FORUMSYS_STR_URL | FORUMSYS_STR_HTML,TRUE);

	# User types
	define("FORUMSYS_USER_REG",	1,TRUE);
	define("FORUMSYS_USER_MOD",	1,TRUE);
	define("FORUMSYS_USER_ADMIN",	1,TRUE);

	# Authentication methods
	define("FORUMSYS_AUTH_HTTP",	1,TRUE);
	define("FORUMSYS_AUTH_SESSION",	2,TRUE);
	define("FORUMSYS_AUTH_COOKIE",	3,TRUE);

	# User-defined fields
	define("FORUMSYS_FLD_USER",	1);
	define("FORUMSYS_FLD_POST",	2);
	define("FORUMSYS_FLD_REPLY",	3);

	# Search types
	define("FORUMSYS_SEARCH_POSTS",	1,TRUE);

	# XML file types
	define("FORUMSYS_XML_POSTS",	1,TRUE);

	# forumsys_build_path() types
	define("FORUMSYS_PATH_ABS",	1,TRUE);
	define("FORUMSYS_PATH_REL",	2,TRUE);
	define("FORUMSYS_PATH_SYS",	3,TRUE);

	# forumsys_rand_str() types
	define("FORUMSYS_RAND_ALNUM",	1,TRUE);	# Alpha-numeric
	define("FORUMSYS_RAND_VIS_NQ",	2,TRUE);	# All visual characters, no quotes

	# Error codes
	define("E_FS_NONE",		0,TRUE);

	# Error codes for user registration
	define("E_FS_USER_USERNAME",	1,TRUE);
	define("E_FS_USER_IN_USE",	2,TRUE);
	define("E_FS_USER_PASSWORD",	4,TRUE);
	define("E_FS_USER_EMAIL",	8,TRUE);

	# Error codes for posting threads
	define("E_FS_POST_SUBJECT",		1,TRUE);
	define("E_FS_POST_BODY",		2,TRUE);
	define("E_FS_POST_NO_BOARD",		4,TRUE);
	define("E_FS_POST_CLOSED_BOARD",	8,TRUE);

	# Error codes for replying
	define("E_FS_REPLY_SUBJECT",		1,TRUE);
	define("E_FS_REPLY_BODY",		2,TRUE);
	define("E_FS_REPLY_NO_POST",		4,TRUE);
	define("E_FS_REPLY_CLOSED_THREAD",	8,TRUE);

	function forumsys_conf()
	{
		static $vars	= array();

		$args		= func_get_args();
		$argc		= func_num_args();

		if ($argc == 2)
		{
			$vars[$args[0]] = $args[1];

		} elseif ($argc != 1) {

			forumsys_handle_error("Wrong number of arguments to forumsys_conf()");
		}

		if (!array_key_exists($args[0],$vars))
			forumsys_handle_error("Configuration value requested before supplied; key: $args[0]");

		return $vars[$args[0]];
	}

	require_once 'CPL/1.0.inc';
	require_once 'forumsys-config.inc';
	require_once 'TimeStamp/1.5.inc';

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
 * NOTE: there is a potential bug here. "Misconfigured" (i.e.,
 * poorly configured) hosts can allow mail servers for domains
 * on which you cannot perform NS lookups, since domain and mx
 * records differ.
 */
				return getHostByName($matches[1]) != $matches[1];
			} else {
				return 0;
			}
		}
	}

	if (!function_exists("unHTMLEntities"))
	{
		/* Concept borrowed from PHP.net */
		function unHTMLEntities($str)
		{
			$table = array_flip(get_html_translation_table(HTML_ENTITIES));

			return strtr($str,$table);
		}
	}

	/*******************************************************
	 * Begin forumsys miscellaneous functions
	 *******************************************************/

	function forumsys_crypt($data)
	{
		return crypt($data,forumsys_conf("crypt_key"));
	}

	function forumsys_t_key_len($type)
	{
		$len = array(2,13);

		switch ($type)
		{
			case 'blowfish':	$len = array(29,60);	break; // 29 or 22?
			case 'md5':		$len = array(15,34);	break;
			case 'ext_des':		$len = array(9,13);	break;
			default:		$len = array(2,13);	break;
		}

		return $len;
	}

	function forumsys_t_gen_key($type = "blowfish")
	{
		$key = "";

		if ($type == "blowfish" && !CRYPT_BLOWFISH)
			$type = "md5";

		if ($type == "md5" && !CRYPT_MD5)
			$type = "ext_des";

		if ($type == "ext_des" && !CRYPT_EXT_DES)
			$type = "default";

		switch ($type)
		{
			case "blowfish":	$key = '$2a$05$'	. forumsys_rand_str(22,FORUMSYS_RAND_ALNUM);	break;
			case "md5":		$key = '$1$'		. forumsys_rand_str(12);				break;
			case "ext_des":		$key =			  forumsys_rand_str(9);				break;
			default:		$key =			  forumsys_rand_str(2);
		}

		return $key;
	}

	function forumsys_rand_str($size,$type = FORUMSYS_RAND_VIS_NQ)
	{
		$random		= "";
		$rand		= "";

		$min		= 0;
		$max		= 0;

		$excluded	= array();

		switch ($type)
		{
			case FORUMSYS_RAND_VIS_NQ:
			{
				$min		= 33;
				$max		= 126;
				$excluded	= array(39,34,92); # ' " and \
				break;
			}

			case FORUMSYS_RAND_ALNUM:
			{
				$min		= 47;
				$max		= 122;
				$excluded	= array(58,59,60,61,62,63,64,91,92,93,94,95,96);
				break;
			}

			default:
				forumsys_handle_error("Invalid type to forumsys_rand_str(); type: $type");
		}

		for ($i = 0; $i < $size; $i++)
		{
			do
			{
				$random = mt_rand($min,$max);

			} while (in_array($random,$excluded));

			$rand .= chr($random);
		}

		return $rand;
	}

	function forumsys_build_path($type = FORUMSYS_PATH_REL)
	{
		$path = "";

		switch ($type)
		{
			case FORUMSYS_PATH_REL: $path = forumsys_conf("web_root") . forumsys_conf("path"); break;
			case FORUMSYS_PATH_SYS: $path = forumsys_conf("sys_root") . forumsys_conf("path"); break;
			case FORUMSYS_PATH_ABS: $path = forumsys_conf("site_uri") . forumsys_conf("path"); break;
			default:
				forumsys_handle_error("Invalid forumsys_build_path() type; type: $type");
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

		@copy($src,$dst)	|| forumsys_handle_error("Cannot copy file; src: $src; dst: $dst");

		@unlink($src)		|| forumsys_handle_error("Cannot unlink() file; file: $src");

		return;
	}

	/* Miscellaenous functions */
	/*
	function forumsys_search($dbh,$type,$offset,$cond)
	{
		switch ($type)
		{
			case FORUMSYS_SEARCH_POSTS: return forumsys_search_posts($dbh,$offset,$cond);
			default:
				forumsys_handle_error("Invalid forumsys_search() type; type: $type");
		}
	}
*/

	function forumsys_search_posts($dbh,$offset,$conds)
	{
		global $forumsys_time_mini_format;

		$s_cond = "";

		foreach ($conds as $name => $val)
		{
			if (!is_array($val))
				$val = array('value' => $val);

			if (preg_match("/ /",$sql))
			{
				if (!@$val["type"])
					$val["type"] = SQL_WILD;

				$words = preg_split("/\s+/",$sql);

				$s_cond .= "(";

				foreach ($words as $word)
					$s_cond .= " $name LIKE '%" . $dbh->prepare_str($word,$val["type"]) . "%' AND";

				$s_cond .= ")";
			} else
				$s_cond .= " $name = '" . $dbh->prepare_str($val["value"],$val["type"]) . "' AND";
		}

		$s_cond = preg_replace("/AND$/","",$s_cond);

		$offset = (int)$offset;

		$total = $dbh->query("	SELECT
						COUNT(*)
					FROM
						forumsys_posts,
						forumsys_users
					WHERE
						$s_cond",DB_COL);

		if ($offset > $total)
			$offset = 0;

		$sql = "	SELECT
					forumsys_posts.id,
					forumsys_posts.user_id,
					forumsys_users.username,
					forumsys_posts.subject,
					MAX(forumsys_posts.post_date) as last_date
				FROM
					forumsys_posts,
					forumsys_users
				WHERE
					/*is_reply = 0 AND*/
					$s_cond
				GROUP BY
					parent_post_id
				ORDER BY
					last_date DESC
				LIMIT
					$offset,
					$forumsys_result_limit";

		$dbh->query($sql,DB_ROWS);

		$menu_output =	forumsys_next_prev_menu
				(
					array
					(
						'url'		=> $_SERVER['PHP_SELF'],
						'offset'	=> $offset,
						'total'		=> $total
					)
				);

		$of	= forumsys_get_of();

		$output	=	$of->table_start() .
				$of->table_head("Subject","Author","Replies","Last Reply");
				$of->table_row(array('colspan' => 4,'value' => $menu_output));
		$path	= forumsys_build_path();

		while ($post = $dbh->fetch_row())
		{
			$ts = new TimeStamp($post["last_date"]);

			$output .=	$of->table_row
					(
						$of->link($post["subject"],	"$path/post.php?post_id=" . $post["id"]),
						$of->link($post["username"],	"$path/profile.php?user_id=" . $post["user_id"]),
						$post["num_replies"],
						$ts->format($forumsys_time_mini_format)
					);
		}

		$output .=	$of->table_row(array('colspan' => 4,'value' => $menu_output));
				$of->table_end();

		return $output;
	}

	/*
	 * forumsys_search_boards() - returns a list of matching
	 * board IDs
	 */
	function forumsys_search_boards($dbh,$prefs,$cri)
	{
		$board_ids = array();

		# Validate criteria
		for ($i = 0; $i < count($cri); $i++)
		{
			if (!forumsys_isr_is_valid("forumsys_boards",$cri[$i]['field'],$cri[$i]['op']))
			{
				# Remove invalid criteria 
				$cri[$i] = $cri[count($cri)-1];
				$i++;
			}
		}

		if (!forumsys_isr_is_valid("forumsys_cats",$prefs['sort_field']))
			$prefs['sort_field'] = "cat_id";

		# Validate prefs
		$t = strToLower($prefs['sort_order']);
		if ($t != 'asc' && $t != 'desc')
			$prefs['sort_order'] = "asc";

		$where = "";

		if (count($cri))
		{
			$where = " WHERE ";

			foreach ($cri as $i_cri)
				$where .= " {$i_cri["field"]} {$i_cri["op"]} {$i_cri["value"]} AND ";
		}

		$sql = "	SELECT
					*
				FROM
					forumsys_boards
				LEFT JOIN
					forumsys_cats
				ON
					cat_id
				$where
				ORDER BY
					forumsys_cats.{$prefs["sort_field"]} {$prefs["sort_order"]}
				LIMIT
					{$prefs["offset"]},
					{$prefs["limit"]}";

		$total = $dbh->query("	SELECT
						COUNT(*)
					FROM
						forumsys_boards
					LEFT JOIN
						forumsys_cats
					ON
						cat_id
					$where",DB_COL);

		return array($dbh->query($sql,DB_ROWS));
	}

	/*
	 * forumsys_search_posts() - returns a list of matching
	 * post IDs
	 */
	function forumsys_search_posts()
	{
	}

	function forumsys_next_prev_menu($of,$prefs)
	{
		$offset		= (int)$prefs["offset"];
		$total		= (int)$prefs["total"];
		$url		= $prefs["url"];
		$lim		= forumsys_conf("result_limit");
		$page_lim	= forumsys_conf("page_limit");
		$output		= "";

		if ($offset >= $lim)
		{
			$output .=	$of->link("First",     $url . "0")		. " | " .
					$of->link("Previous",  $url . ($offset - $lim))	. " | ";
		} else {
			$output .=	"First | Previous | ";
		}

		$current_index	= floor($offset	/ $lim);
		$last_index	= ceil($total	/ $lim);

		if ($current_index < $page_lim / 2)
		{
			/*
			 * The "left boundary" exists when we can't evenly distribute
			 * the "previous pages'" hyperlinks
			 */
			$start	= 0;

		} elseif ($current_index > $last_index - ceil($page_lim / 2)) {

			/*
			 * The "right boundary" exists when we can't evenly distribute
			 * the "next pages'" hyperlinks
			 */

			$start	= $last_index - $page_lim;
		} else {
			$start	= $current_index - floor($page_lim / 2);
		}

		for ($i = $start; $i < $start + $page_lim && $i < $last_index; $i++)
		{
			if ($i == $current_index)
			{
				$output .= $of->strong($i + 1) . " | ";
			} else {
				$output .= $of->link($i + 1,$url . ($i * $lim)) . " | ";
			}
		}

		if ($offset + $lim < $total)
		{
			$output .=	$of->link("Next",$url . ($offset + $lim)) . " | " .
					$of->link("Last",$url . ($total - $lim));
		} else {
			$output .=	"Next | Last";
		}

		return $output;
	}

	function forumsys_get_dbh()
	{
		static $dbh = array();

		$dbh_type	= forumsys_conf("dbh_type");

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
		static $count = 0;
		global $php_errormsg;

		$msg .= "; Date: " . date("F d, Y h:i:s A") .
			"; File: " . $_SERVER['PHP_SELF'];

		if (ini_get("track_errors"))
			$msg .= "; PHP Error: " . $php_errormsg;

		$msg .= "\n";

		if (forumsys_conf("trigger_errors"))
			trigger_error($msg,E_USER_ERROR);

		if ($log = forumsys_conf("error_log"))
			if ($fp = @fopen($log,"a"))
			{
				fputs($fp,$msg);
				fclose($fp);
			}

		/* forumsys_mail() may recursively throw an error */
		if (forumsys_conf("mail_errors"))
			@mail
			(
				forumsys_conf("admin_email"),
				forumsys_conf("site_name") . " Forumsys Error",
				"A Forumsys error has occured:\n\n$msg",
				"From: " . forumsys_conf("admin_email")
			);

		$of = forumsys_get_of();

		if (forumsys_conf("show_errors"))
		{
			if ($count++)
			{
				echo	"The following error occured while processing that directive: $msg";
			} else {
				echo	forumsys_get_template("header"),
					$of->header("Fatal Error"),
					$of->p("The following error occured while processing that directive: $msg"),
					forumsys_get_template("footer");
			}
		} else {
			if ($count)
			{
				echo	"An error has occurred. Please contact the site administrator and report this error.";
			} else {
				echo	forumsys_get_template("header"),
					$of->header("Fatal Error"),
					$of->p("An error has occurred. Please contact the site administrator and report this error"),
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
			$path   = $_SERVER["PHP_SELF"];

			$path   = preg_replace("![^/]+$!","",$path);

			$url    = forumsys_conf("site_uri") . $path . $url;
		}

		header("Location: $url");

	      	exit(0);
	}

	function forumsys_mail($to,$from,$subject,$body)
	{
		global $php_errormsg;

		if (!@mail($to,$subject,$body,"From: $from"))
			forumsys_handle_error("Error in mail()");

		return;
	}

	/*
	 * This temporary function is just used to clean up attributes
	 * found by forumsys_str_parse()
	 */
	function forumsys_t_clean_attr($attr)
	{
		# Attributes will be matched with their delimiters (" or ')
		$attr = preg_replace("/^&quot;|^'|&quot;$|'$/","",$attr);

		# Attributes will have been subject to htmlEntities()
		$attr = unHTMLEntities($attr);

		return $attr;
	}

	/*
	 * This temporary function is just used to check potentially
	 * arbitrarily-protocoled URIs for disallowed protocols
	 */
	function forumsys_t_check_proto($url)
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
								forumsys_t_check_proto(forumsys_t_clean_attr("$3"))

								# Else it is good
								: TRUE
							) ?

							# Formatted attribute
							" $2=\"" . forumsys_t_clean_attr("$3") . "\""

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

	/* Cache functions */
	function forumsys_cache_expire()
	{
		global $php_errormsg;

		$args		= func_get_args();
		$id		= join('-',$args);
		$cache_dir	= forumsys_conf("cache_dir");

		if (!@touch("$cache_dir/info-$id"))
			forumsys_handle_error("Cannot touch() file; file: $cache_dir/info-$id");

		return;
	}

	function forumsys_cache_delete()
	{
		$args		= func_get_args();
		$id		= join('-',$args);
		$cache_dir	= forumsys_conf("cache_dir");

		/* This data might not be cached */
		if (file_exists("$cache_dir/cache-$id"))
			forumsys_unlink("$cache_dir/cache-$id");

		if (file_exists("$cache_dir/info-$id"))
			forumsys_unlink("$cache_dir/info-$id");

		return;
	}

	function forumsys_unlink($file)
	{
		global $php_errormsg;

		if (!@unlink($file))
			forumsys_handle_error("Cannot unlink() file; file: $file");

		return;
	}

	function forumsys_cache_add_info()
	{
		global $php_errormsg;

		$args		= func_get_args();
		$data		= array_shift($args);
		$id		= join('-',$args);
		$cache_dir	= forumsys_conf("cache_dir");

		$fp = @fopen("$cache_dir/info-$id","a");

		if (!$fp)
			forumsys_handle_error("Cannot open file; file: $cache_dir/info-$id");

		fputs($fp,$data . "\n");

		fclose($fp);

		return;
	}

	function forumsys_cache_remove_info()
	{
		global $php_errormsg;

		$args		= func_get_args();
		$data		= array_shift($args);
		$id		= join('-',$args);
		$cache_dir	= forumsys_conf("cache_dir");
		$temp_dir	= forumsys_conf("temp_dir");

		/* For allowing removal of multiple pieces of data */
		if (!is_array($data))
			$data = array($data);

		/* Incoming data */
		$fp_in	= @fopen("$cache_dir/info-$id","r");

		if (!$fp_in)
			forumsys_handle_error("Cannot open file; file=$cache_dir/info-$id");

		/* Temporary storage */
		$f_temp	= tempnam($temp_dir,"ns-info");

		if (!$f_temp)
			forumsys_handle_error("Cannot create temporary filename with tempnam()");

		/* Outgoing data */
		$fp_out	= @fopen($f_temp,"w");

		if (!$fp_out)
			forumsys_handle_error("Cannot open file; file: $f_temp");

		# Keep all data except our exclusion
		while ($line = fgets($fp_in,FORUMSYS_BUF_SIZE))
			if (!in_array(chop($line),$data))
				fputs($fp_out,$line);

		fclose($fp_out);
		fclose($fp_in);

		forumsys_move_file($f_temp,"$cache_dir/info-$id");

		return;
	}

	function forumsys_cache_is_fresh()
	{
		static $cache		= array();

		static $cache_dir;
		static $templates_dir;
		$args			= func_get_args();
		$type			= array_shift($args);
		$id			= join('-',$args);

		if (!$cache_dir)
			$cache_dir	= forumsys_conf("cache_dir");

		if (!$templates_dir)
			$templates_dir	= forumsys_conf("templates_dir");

		# Before we do anything, make sure the file is cached
		if (!file_exists("$cache_dir/cache-$id"))
			return FALSE;

		# Grab the cache's last modification timestamp
		$mod_time	= @filemtime("$cache_dir/cache-$id");

		# Cache last mod time for templates
		if (!array_key_exists("mtime_struct_$type",$cache))
		{
			$cache["mtime_struct_$type"]	= filemtime("$templates_dir/$type.inc");
			$cache["mtime_content_$type"]	= filemtime("$templates_dir/$type.t");
			$cache["dep_$type"]		= @file("$templates_dir/$type.dep");

			$deps				= array();

			# Compile dependencies
			if (is_array($cache["dep_$type"]))
			{
				foreach ($cache["dep_$type"] as $dep)
				{
					# Skip comments/blank lines
					if (preg_match("/^\s*#|^\s*$/",$dep))
						continue;

					# Syntax: rule:arg
					preg_match("/^(\w+):(.*)$/",$dep,$matches);

					# Expand variables
					$matches[2] =	preg_replace
							(
								"/
									{?			# Optional { ... } delimiter
									(
										\\\$		# Dollar
										[a-zA-Z_]	# First char
										[a-zA-Z_0-9]+	# Subsequent chars
										(?:
											\[	# Subscript
											([\"'])	# Quotes
											.*?	# Index
											\\2	# Matching quote
											\]
										)*
									)
									}?
								/xe",
								"$1",
								$matches[2]
							);

					array_push($deps,array('type' => $matches[1],'arg' => $matches[2]));
				}
			}

			$cache["dep_$type"]		= $deps;
		}

		# Check if the templates have changed
		if
		(
			$cache["mtime_struct_$type"] > $mod_time
			||
			$cache["mtime_content_$type"] > $mod_time
		)
			return FALSE;

		# Make sure the dependencies haven't expired
		foreach ($cache["dep_$type"] as $dep)
		{
			switch ($dep["type"])
			{
				# Check last mod time
				case "file":
					if (@filemtime($dep['arg']) > $mod_time)
						return FALSE;

					break;
			}
		}

		# It must be fresh then...
		return TRUE;
	}

	if (!function_exists('file_dump'))
	{
		function file_dump($file,$data)
		{
			global $php_errormsg;

			$fp = @fopen($file,"w");

			if (!$fp)
				forumsys_handle_error("Unable to write file; file: $file");

			fputs($fp,$data);

			fclose($fp);

			return $data;
		}
	}

	if (!function_exists('file_slurp'))
	{
		function file_slurp($file)
		{
			global $php_errormsg;

			$fp = @fopen($file,"r");

			if (!$fp)
				forumsys_handle_error("Unable to read file; file: $file");

			$size = @filesize($file);

			$data = fread($fp,$size);

			fclose($fp);

			return $data;
		}
	}

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






















	/* XML functions */
	function forumsys_xml_add()
	{
	}

	function forumsys_xml_remove()
	{
	}

	function forumsys_xml_write()
	{
	}

	function forumsys_xml_update()
	{
	}

	function forumsys_xml_setup()
	{
	}

	/* User functions */
	function forumsys_user_add($dbh,$user)
	{

	}

	function forumsys_user_remove($dbh,$user)
	{
	}

	function forumsys_user_update($dbh,$user)
	{
	}

	function forumsys_user_get($dbh,$user_id)
	{
	}

	/* Category functions */
	function forumsys_cat_add($dbh,$cat)
	{
	}

	function forumsys_cat_remove($dbh,$cat)
	{
	}

	function forumsys_cat_update($dbh,$cat)
	{
	}

	function forumsys_cat_get($dbh,$cat_id)
	{
	}

	/* Board Functions*/
	function forumsys_board_add($dbh,$board)
	{
	}

	function forumsys_board_remove($dbh,$board)
	{
	}

	function forumsys_board_update($dbh,$board)
	{
	}

	function forumsys_board_get($dbh,$board_id)
	{
		$board_id = (int)$board_id;

		return $dbh->query("	SELECT
						*
					FROM
						forumsys_boards
					WHERE
						board_id = $board_id");
	}

	/* Post functions */
	function forumsys_post_add($dbh,$post)
	{

	}

	function forumsys_post_remove($dbh,$post)
	{
	}

	function forumsys_post_update($dbh,$post)
	{
	}

	function forumsys_post_get($dbh,$post_id)
	{
	}

	/* Reply functions */
	function forumsys_reply_add($dbh,$reply)
	{
	}

	function forumsys_reply_remove($dbh,$reply)
	{
	}

	function forumsys_reply_update($dbh,$reply)
	{
	}

	function forumsys_reply_get($dbh,$reply_id)
	{
	}

	/* Session functions */
	function forumsys_is_logged_in()
	{
	}

	function forumsys_session_start()
	{
	}

	function forumsys_login()
	{
	}

	function forumsys_logout()
	{
	}

	/* Bulk-output functions */
	function forumsys_of_login()
	{
	}

	function forumsys_of_actions()
	{
	}

	/* Internal structure representation functions */
	/*
	 * Note that this function totally sucks because it
	 * must be kept up to date with the internal database
	 * representation of various structures used by forumsys
	 * as well as keep a list of available relationships
	 * implored by the database
	 */
	function forumsys_isr_is_valid($table,$field,$op = NULL)
	{
		$int_ops 	= array('<','<=','>','>=','=','<>');
		$str_ops 	= array('=','like');
		$date_ops	= array('<','>');
		$bool_ops	= array('=');

		switch ($table)
		{
			case 'forumsys_posts':
			{
				switch ($field)
				{
					case 'post_id':
					case 'user_id':
					case 'board_id':
					case 'cat_id':
					case 'reply_id':
					case 'parent_post_id':
						return in_array($rel,$int_ops);
					case 'subject':
					case 'message':
						return in_array($rel,$str_ops);
					case 'post_date':
					case 'mod_date':
						return in_array($rel,$date_ops);
					case 'email_replies':
					case 'is_closed':
					case 'is_private':
					case 'is_notice':
						return in_array($rel,$bool_ops);
					default:
						return FALSE;
				}

				break;
			}

			case 'forumsys_boards':
			{
				switch ($field)
				{
					case 'board_id':
					case 'cat_id':
						return in_array($rel,$int_ops);
					case 'name':
					case 'description':
						return in_array($rel,$str_ops);
					default:
						return FALSE;
				}

				break;
			}

			case 'forumsys_cats':
			{
				switch ($field)
				{
					case 'cat_id':
						return in_array($rel,$int_ops);
					case 'name':
					case 'description':
						return in_array($rel,$str_ops);
					default:
						return FALSE;
				}
				
				break;
			}
			
			default:
				return FALSE;
		}

		/* default to invalidity */
		return FALSE;
	}

	/* Custom field functions */
	function forumsys_fields_validate()
	{
	}

	function forumsys_fields_update()
	{
	}

	function forumsys_fields_write_cache()
	{
	}

	function forumsys_fields_write_db()
	{
	}

	if (!function_exists("strip_recursive_slashes"))
	{
		function strip_recursive_slashes(&$data)
		{
			if (!is_array($data))
				return;

			foreach ($data as $key => $val)
			{
				if (is_array($val))
					strip_recursive_slashes($data[$key]);
				else
					$data[$key] = stripSlashes($val);
			}

			return;
		}
	}

	/* Only doing this to encapsulate variables */
	if (ini_get('magic_quotes_gpc'))
	{
		strip_recursive_slashes($_POST);
		strip_recursive_slashes($_GET);
		strip_recursive_slashes($_REQUEST);
		strip_recursive_slashes($_COOKIE);
		strip_recursive_slashes($_SESSION);
	}

	ini_set("magic_quotes_runtime",FALSE);
?>
