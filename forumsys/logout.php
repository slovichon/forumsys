<?php
	require_once "forumsys.inc";

	$forumsys_dbh	= forumsys_get_dbh();
	$forumsys_of	= forumsys_get_of();

	if (forumsys_is_logged_in($forumsys_dbh))
	{
		forumsys_log_out();

		echo	forumsys_get_template("header"),
			$forumsys_of->header("Logged Out"),
			$forumsys_of->p("You have successfully logged out."),
			$forumsys_of->p($forumsys_of->link("Log Back In",forumsys_build_path() . "/login.php")),
			forumsys_get_template("footer");
	} else {
		echo	forumsys_get_template("header"),
			$forumsys_of->header("Error"),
			$forumsys_of->p("You must log in before you can log out."),
			forumsys_of_login($forumsys_of),
			forumsys_get_template("footer");
	}
?>
