<?php
	/* this can NEVER be cached */
	$NO_CACHE	= TRUE;
	$cat		= $args[1];
	$url		= forumsys_build_path();

	$cat_name	= $cat['name'];
	$cat_url	= "$url/searchposts.php?";

	/*
	 * We shouldn't get this far if a cat doesn't exist
	 * (however, will an extra sanity check hurt?
	 */
	return TRUE;
?>
