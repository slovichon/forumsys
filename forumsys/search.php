<?php
	/*
	 * Different pages:
	 *	(none) - list categories/boards
	 *	cat_id - list boards under certain category
	 */

	require_once "forumsys.inc";

	function forumsys_t_of_board($dbh,$of,$board)
	{
		static 	$last_board,
			$mods,
			$last_cat,
			$count = 0;

		$out = "";

		if (!is_array($mods))
			$mods = array();

		if (!is_array($last_cat))
			$last_cat = array();

		/* check if last cat matched this one */
		if
		(
			$board['cat_id'] == $last_board['cat_id']
		)
		{
			/* previous and current cats match, must be a new board on the same cat */
		} else {
			/* cat changed, output last cat */
			$out .= forumsys_get_template('cat-list',$last_board['cat_id']);
		}

		/* check if last board matched this one */
		if
		(
			$board['board_id'] 		== $last_board['board_id'] &&
			$board['cat_id'] 		== $last_board['cat_id']	
		)
		{
			/* this board is a duplicate with a new mod, so add the mod to the list */
			$mods[] = $board['mod_id'];
		} else {
			/* board changed, output last board */
			$last_board['mods'] = $mods;

			$out .= forumsys_get_template('board-list',$last_board);

			/* clear moderators */
			$mods = array();
		}

		$last_board = $board;

		return $out;
	}
	
	$forumsys_dbh			= forumsys_get_dbh();
	$forumsys_of			= forumsys_get_of();
	$forumsys_path			= forumsys_build_path();
	$forumsys_template		= @$_REQUEST['template'];
	$forumsys_criteria_domain	= @$_REQUEST['criteria_domain'];
	$forumsys_criteria_name		= @$_REQUEST['criteria_name'];
	$forumsys_criteria_relationship	= @$_REQUEST['criteria_relationship'];
	$forumsys_criteria_value	= @$_REQUEST['criteria_value'];
	$forumsys_result_limit		= forumsys_conf('result_limit');

	/* validate types */
	if (!is_array($forumsys_criteria_name))
		$forumsys_criteria_name = array();

	if (!is_array($forumsys_criteria_relationship))
		$forumsys_criteria_relationship = array();

	if (!is_array($forumsys_criteria_value))
		$forumsys_criteria_value = array();

	/* default to listing categories/boards */
	if (!in_array($forumsys_criteria_domain,array('forumsys_posts','forumsys_boards','forumsys_cats')))
		$forumsys_criteria_domain = 'forumsys_cats';

	/* default to listing posts (if listing posts) */
	if (!in_array($forumsys_template,array('post-list','post-show')))
		$forumsys_template = 'post-list';

	/* build criteria list */
	$forumsys_criteria = array();
	foreach ($forumsys_criteria_name as $forumsys__i_key => $forumsys__i_name)
		if
		(
			forumsys_isr_is_valid
			(
				$forumsys_criteria_domain,
				@$forumsys_criteria_relationship[$forumsys__i_key],
				@$forumsys_criteria_name[$forumsys_i_key],
				@$forumsys_criteria_value[$forumsys_i_key]
			)
		)
			$forumsys_criteria[$forumsys__i_name] =	array
								(
									'rel'	=> @$forumsys_criteria_relationship[$forumsys__i_key],
									'val'	=> @$forumsys_criteria_value[$forumsys__i_key]
								);

	switch ($forumsys_criteria_domain)
	{
		case 'forumsys_posts': /* list/view posts */
		{
			$forumsys_sql = "";

			$forumsys_dbh->query($forumsys_sql,DB_ROWS);

			while ($forumsys_ = $forumsys_dbh->fetch_row())
			{
			}

			break;
		}

		case 'forumsys_boards': /* list posts */
		{
			$forumsys_sql = "";
			break;
		}

		case 'forumsys_cats': /* list categories/boards */
		default:
		{
			$forumsys_sql_where = " WHERE 1=1 ";

			foreach ($forumsys_criteria as $forumsys__i_name => $forumsys__i_cri)
				$forumsys_sql_where .= " AND $forumsys__i_name {$forumsys__i_cri['rel']} '".$forumsys_dbh->prepare_str($forumsys__i_cri['val'],SQL_REG)."' AND ";

#			$forumsys_sql_where = preg_replace("/(?:AND|OR) $/","",$forumsys_sql_where);

			$forumsys_sql = "	SELECT
							*
						FROM
							forumsys_boards
						LEFT JOIN
							forumsys_cats
						ON
							forumsys_boards.cat_id = forumsys_cats.cat_id
						LEFT JOIN
							forumsys_moderators
						ON
							forumsys_boards.board_id = forumsys_moderators.board_id
						$forumsys_sql_where
						";
/*
			$forumsys_sql = "	SELECT
							forumsys_boards.board_id,
							forumsys_boards.cat_id,
							forumsys_boards.name,
							forumsys_boards.description
						FROM
							forumsys_boards,
							forumsys_cats
						$forumsys_sql_where
						ORDER BY
							forumsys_boards.cat_id";
*/
			$forumsys_dbh->query($forumsys_sql,DB_ROWS);

			echo	forumsys_get_template('header'),
				$forumsys_of->header('Boards'),
				$forumsys_of->table_start(array('class'=>"forumsysTable")),
				$forumsys_of->table_row
				(
					array('class'=>"forumsysHeader",'value'=>'Board'),
					array('class'=>"forumsysHeader",'value'=>'Threads'),
					array('class'=>"forumsysHeader",'value'=>'Posts'),
					array('class'=>"forumsysHeader",'value'=>'Last Post'),
					array('class'=>"forumsysHeader",'value'=>'Moderator(s)')
				);

			$forumsys_count = 0;

			while ($forumsys_board = $forumsys_dbh->fetch_row())
			{
				echo forumsys_t_of_board($forumsys_dbh,$forumsys_of,$forumsys_board);
				$forumsys_count++;
			}

			echo forumsys_t_of_board($forumsys_dbh,$forumsys_of,NULL);

			if ($forumsys_count == 0)
				echo $forumsys_of->table_row(array('colspan'=>4,'class'=>"forumsysData",'value'=>"No posts are available."));

			echo	$forumsys_of->table_end(),
				forumsys_get_template('footer');

			break;
		}
	}
/*
####################################################################################


	$forumsys_boards_sql	= "";
	$forumsys_mod_sql	= "";
	$forumsys_cat_sql	= "	SELECT
						*
					FROM
						forumsys_cats
					ORDER BY
						id DESC";

	if ($forumsys_cat_id)
	{
		$forumsys_boards_sql	= "	SELECT
							*
						FROM
							boards
						WHERE
							cat_id = $forumsys_cat_id
						ORDER BY
							board_id DESC";

		$forumsys_mod_sql	= "	SELECT
							forumsys_moderators.mod_id,
							forumsys_moderators.board_id,
							forumsys_users.username
						FROM
							forumsys_moderators,
							forumsys_users
						WHERE
							forumsys_moderators.cat_id	= $forumsys_cat_id		AND
							forumsys_moderators.mod_id	= forumsys_users.user_id
						ORDER BY
							forumsys_moderators.board_id	DESC";
	} else {
		$forumsys_boards_sql	= "	SELECT
							*
						FROM
							forumsys_boards,
							forumsys_posts
						ORDER BY
							forumsys_boards.cat_id DESC";

		$forumsys_mod_sql	= "	SELECT
							forumsys_moderators.mod_id,
							forumsys_moderators.board_id,
							forumsys_users.username
						FROM
							forumsys_moderators,
							forumsys_users
						WHERE
							forumsys_moderators.mod_id = forumsys_users.user_id
						ORDER BY
							forumsys_moderators.board_id	DESC";

	}

	# Capture the boards results 
	$forumsys_dbh->query($forumsys_boards_sql,DB_ROWS);
	$forumsys_result_boards	= $forumsys_dbh->result_id;

	# Prepare first board
	if (!$forumsys_cat_id)
		$forumsys_board = $forumsys_dbh->fetch_row();

	# Capture the moderator results 
	$forumsys_dbh->query($forumsys_mod_sql,DB_ROWS);
	$forumsys_result_mod	= $forumsys_dbh->result_id;

	# Number-of-categories counter 
	$forumsys_cat_count	= 0;

	# Capture the categories result 
	$forumsys_dbh->query($forumsys_cat_sql,DB_ROWS);
	$forumsys_result_cat	= $forumsys_dbh->result_id;

	echo	forumsys_get_template("header"),
		$forumsys_of->table_start
		(
			array
			(
				'class'	=> "forumsysTable",
				'cols'	=>	array
						(
							array('width' => '50%'),
							array('width' => '5%'),
							array('width' => '5%'),
							array('width' => '20%'),
							array('width' => '20%')
						)
			)
		),
			$forumsys_of->table_row
			(
				array('class' => "forumsysHeader",'value' => "Forum"),
				array('class' => "forumsysHeader",'value' => "Posts"),
				array('class' => "forumsysHeader",'value' => "Threads"),
				array('class' => "forumsysHeader",'value' => "Last Post"),
				array('class' => "forumsysHeader",'value' => "Moderator(s)")
			);

	while ($forumsys_cat = $forumsys_dbh->fetch_row())
	{
		$forumsys_cat_count++;
		$forumsys_board_count = 0;

		echo $forumsys_of->table_row(array('colspan' => 5,'class' => "forumsysHeader",'value' => $forumsys_cat["name"]));

		if ($forumsys_cat_id && $forumsys_cat_id == $forumsys_cat["id"])
		{
			# List specific boards
			while ($forumsys_board = $forumsys_dbh->fetch_row())
			{
				$forumsys_board_count++;

				echo forumsys_t_dump_board($forumsys_of,$forumsys_dbh,$forumsys_board,$forumsys_result_mod);
			}
		} else {
			# List all boards
			$forumsys_dbh->result_id = $forumsys_result_boards;

			do
			{
				if ($forumsys_board["cat_id"] != $forumsys_cat["id"])
					break;

				$forumsys_board_count++;

				echo forumsys_t_dump_board($forumsys_of,$forumsys_dbh,$forumsys_board,$forumsys_result_mod);

			} while ($forumsys_board = $forumsys_dbh->fetch_row());
		}

		if (!$forumsys_board_count)
			echo $forumsys_of->table_row(array('colspan' => 5,'class' => "forumsysData1",'value' => "No boards are available at this time."));

		$forumsys_dbh->result_id = $forumsys_result_cat;
	}

	if (!$forumsys_cat_count)
		echo $forumsys_of->table_row(array('colspan' => 5,'class' => "forumsysData1",'value' => "No categories are available at this time."));

	echo	$forumsys_of->table_end(),
		forumsys_get_template("footer");

###############################################################################

	$forumsys_board_id	= (int)@$REQUEST["board_id"];

	if (!$forumsys_board_id)
		forumsys_redirect("boards.php");

	$dbh			= new DBH();
	$forumsys_board		= forumsys_board_get($dbh,$forumsys_board_id);

	if (!is_array($forumsys_board))
		forumsys_redirect("boards.php");

	echo	forumsys_get_template("header"),
		OF::header($forumsys_board["name"]),
		forumsys_search
		(
			$dbh,
			FORUMSYS_SEARCH_POSTS,
			@$REQUEST["offset"],
			array('forumsys_posts.board_id' => $forumsys_board["id"])
		),
		forumsys_get_template("footer");

#################################################################################

	$forumsys_post_id	= (int)@$REQUEST["post_id"];

	if (!$forumsys_post_id)
		forumsys_redirect("boards.php");

	$dbh = new DBH();

	$dbh->query("	SELECT
				*
			FROM
				forumsys_posts
			WHERE
				post_id = $forumsys_post_id");

	if (!is_array($forumsys_post))
		forumsys_redirect("boards.php");

	echo	forumsys_get_template("header"),
		OF::header($forumsys_board["name"]),
		forumsys_search
		(
			$dbh,
			FORUMSYS_SEARCH_POSTS,
			@$REQUEST["offset"],
			array('forumsys_posts.post_id' => $forumsys_post["id"])
		),
		forumsys_get_template("footer");
?>
*/
?>
