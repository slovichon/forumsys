<?php
	/*
	 * Forumsys
	 * By Jared Yanovich <jaredy@closeedge.net>
	 * Monday, March 17, 2003 11:39:00 AM
	 *
	 * of.inc - bulk output/formatting routines
	 */
	function forumsys_of_nav_menu($of,$prefs)
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

	function forumsys_of_login()
	{
	}

	function forumsys_of_actions()
	{
	}
?>
