<?php
	require_once "forumsys.inc";

	$forumsys_of = forumsys_get_of();

	if (@$_POST["forumsys_submitted"])
	{
		$forumsys_dbh = forumsys_get_dbh();

		# Allow all users to log in
		list
		(
			$forumsys_user_id,
			$forumsys_user_type

		) = forumsys_log_in($forumsys_dbh,FORUMSYS_USER_REG);

		# Gather filename disregarding potentially useless
		# (and dangerous) appendages
		$forumsys_redirect = forumsys_conf("sys_root") . preg_replace("/[?#].*$/","",@$_POST["forumsys_redir"]);

		if (@$_POST["forumsys_redir"] && file_exists($forumsys_redirect))
			forumsys_redirect($_POST["forumsys_redir"]);

		forumsys_redirect(forumsys_build_path(FORUMSYS_PATH_ABS) . "/admin/index.php");
	} else {
		echo	forumsys_get_template("header"),
			$forumsys_of->header("Log In"),
			forumsys_of_login($forumsys_of),
			forumsys_get_template("footer");
	}
?>
