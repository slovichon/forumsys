<?php
	/*
	 * Forumsys
	 * By Jared Yanovich <jaredy@closeedge.net>
	 * Monday, March 17, 2003 10:57:00 AM
	 *
	 * board.inc - board routines
	 */


	# Returns a list of matching board IDs
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
?>
