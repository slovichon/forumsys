<?php
	/* this can NEVER be cached */
	$NO_CACHE		= TRUE;
	$board			= $args[1];
	$url			= forumsys_build_path();

	$board_url		= "$url/searchboards.php?";
	$board_name		= $board['name'];
	$board_description	= $board['description'];
	$num_posts		= $board['num_posts'];
	$num_threads		= $board['num_threads'];
	$last_post_url		= "$url/searchposts.php?";
	$last_post_subject	= $board['last_post']['subject'];
	$last_post_author	= $board['last_post']['author'];
	$last_post_author_url	= "$url/profile.php?";
	$moderators		= join(', ',$board['mods']);

	/*
	 * We shouldn't get this far if a cat doesn't exist
	 * (however, will an extra sanity check hurt?
	 */
	return TRUE;
?>
