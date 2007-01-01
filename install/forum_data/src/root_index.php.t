<?php
/**
* copyright            : (C) 2001-2007 Advanced Internet Designs Inc.
* email                : forum@prohost.org
* $Id: root_index.php.t,v 1.46 2007/01/01 18:23:46 hackie Exp $
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; version 2 of the License.
**/

	require('./GLOBALS.php');

/*{PRE_HTML_PHP}*/
/*{POST_HTML_PHP}*/

	fud_use('err.inc');

	/* before we go on, we need to do some very basic activation checks */
	if (!($FUD_OPT_1 & 1)) {
		fud_use('errmsg.inc');
		exit($DISABLED_REASON . __fud_ecore_adm_login_msg);
	}
	if (!$FORUM_TITLE && @file_exists($WWW_ROOT_DISK.'install.php')) {
		fud_use('errmsg.inc');
	        exit(__fud_e_install_script_present_error);
	}

	if ($FUD_OPT_2 & 16384 && $t != 'getfile') {
		ob_start(array('ob_gzhandler', (int)$PHP_COMPRESSION_LEVEL));
	}

	if ($t == 'rview') {
		if (isset($_GET['th']) || isset($_GET['goto'])) {
			$t = $_GET['t'] = d_thread_view;
		} else if (isset($_GET['frm_id'])) {
			$t = $_GET['t'] = t_thread_view;
		} else {
			$t = $_GET['t'] = 'index';
		}
	} else if (ereg('[^a-z_]', $t) || !@file_exists($WWW_ROOT_DISK . fud_theme . $t . '.php')) {
		$t = 'index';
	}

	if ($FUD_OPT_2 & 524288 && isset($_COOKIE[$COOKIE_NAME.'1']) && $t != 'error') {
		fud_use('errmsg.inc');
		exit(__fud_banned__);
	}

	/* this is needed to determine what extension to use for alpha-transparency images */
	if (!empty($_SERVER['HTTP_USER_AGENT']) && 
		strpos($_SERVER['HTTP_USER_AGENT'], 'MSIE') !== false && 
		strpos($_SERVER['HTTP_USER_AGENT'], 'Opera') === false) {
		define('img_ext', '.gif');
	} else {
		define('img_ext', '.png');
	}

	require($WWW_ROOT_DISK . fud_theme . $t . '.php');
?>