<?php
	/*
	 * Forumsys
	 * By Jared Yanovich <jaredy@closeedge.net>
	 * Monday, March 17, 2003 11:16:56 AM
	 *
	 * crypt.inc - cryptographic routines
	 */

	# forumsys_rand_str() types
	define("FORUMSYS_RAND_ALNUM",	1,TRUE);	# Alpha-numeric
	define("FORUMSYS_RAND_VIS_NQ",	2,TRUE);	# All visual characters, no quotes

	function forumsys_crypt($data)
	{
		static $crypt_key = NULL;

		if ($crypt_key === NULL)
			$crypt_key = forumsys_conf('crypt_key');

		return crypt($data,$crypt_key);
	}

	function forumsys_crypt_key_len($type)
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

	function forumsys_crypt_gen_key($type = "blowfish")
	{
		$key = "";

		if ($type == "blowfish"	&& !CRYPT_BLOWFISH)	$type = "md5";
		if ($type == "md5"	&& !CRYPT_MD5)		$type = "ext_des";
		if ($type == "ext_des"	&& !CRYPT_EXT_DES)	$type = "default";

		switch ($type)
		{
			case "blowfish":	$key = '$2a$05$'	. forumsys_rand_str(22,FORUMSYS_RAND_ALNUM);	break;
			case "md5":		$key = '$1$'		. forumsys_rand_str(12);			break;
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
				forumsys_handle_error("forumsys_rand_str(): invalid type; type: $type");
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
?>
