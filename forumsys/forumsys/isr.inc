<?php
	/*
	 * Forumsys
	 * By Jared Yanovich <jaredy@closeedge.net>
	 * Monday, March 17, 2003 11:40:28 AM
	 *
	 * isr.inc - internal structure representation
	 * routines
	 *
	 * Note that these function totally suck because they
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
?>
