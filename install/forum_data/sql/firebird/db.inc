<?php
/**
* copyright            : (C) 2001-2010 Advanced Internet Designs Inc.
* email                : forum@prohost.org
* $Id$
*
* This program is free software; you can redistribute it and/or modify it 
* under the terms of the GNU General Public License as published by the 
* Free Software Foundation; version 2 of the License. 
**/
#define('fud_query_stats', 1);

if (!defined('fud_sql_lnk')) {
	$connect_func = $GLOBALS['FUD_OPT_1'] & 256 ? 'ibase_pconnect' : 'ibase_connect';

	$conn = $connect_func($GLOBALS['DBHOST'] .':'. $GLOBALS['DBHOST_DBNAME'], $GLOBALS['DBHOST_USER'], $GLOBALS['DBHOST_PASSWORD']) or fud_sql_error_handler("Initiating $connect_func", ibase_errmsg(), ibase_errcode(), "Unknown");
	define('fud_sql_lnk', $conn);
	// ibase_select_db($GLOBALS['DBHOST_DBNAME'], fud_sql_lnk) or fud_sql_error_handler("Opening database ".$GLOBALS['DBHOST_DBNAME'], ibase_errmsg(), ibase_errcode(), db_version());
	// ibase_query("SET NAMES 'utf8' COLLATE 'utf8_general_ci'");

	define('__dbtype__', 'mysql');
	define('__FUD_SQL_CONCAT__', 'CONCAT');
}

function db_version()
{
	if (!defined('__FUD_SQL_VERSION__')) {
		$ver = ibase_fetch_row(ibase_query('SELECT rdb$get_context(\'SYSTEM\', \'ENGINE_VERSION\') from rdb$database'));
		define('__FUD_SQL_VERSION__', $ver[0]);
	}
	return __FUD_SQL_VERSION__;
}

function db_lock($tables)
{
	if (!empty($GLOBALS['__DB_INC_INTERNALS__']['db_locked'])) {
		fud_sql_error_handler("Recursive Lock", "internal", "internal", db_version());
	} else {
		// q('LOCK TABLES '.$tables);
		$GLOBALS['__DB_INC_INTERNALS__']['db_locked'] = 1;
	}
}

function db_unlock()
{
	if (empty($GLOBALS['__DB_INC_INTERNALS__']['db_locked'])) {
		unset($GLOBALS['__DB_INC_INTERNALS__']['db_locked']);
		fud_sql_error_handler("DB_UNLOCK: no previous lock established", "internal", "internal", db_version());
	}
	
	if (--$GLOBALS['__DB_INC_INTERNALS__']['db_locked'] < 0) {
		unset($GLOBALS['__DB_INC_INTERNALS__']['db_locked']);
		fud_sql_error_handler("DB_UNLOCK: unlock overcalled", "internal", "internal", db_version());
	}
	unset($GLOBALS['__DB_INC_INTERNALS__']['db_locked']);
	q('UNLOCK TABLES');
}

function db_locked()
{
	return isset($GLOBALS['__DB_INC_INTERNALS__']['db_locked']);
}

function db_affected()
{
	return ibase_affected_rows(fud_sql_lnk);	
}

if (!defined('fud_query_stats')) {
	function q($query)
	{
		$r = ibase_query($query) or fud_sql_error_handler($query, ibase_errmsg(), ibase_errcode(), db_version());
		return $r;
	}
	function uq($query)
	{
		$r = ibase_query($query) or fud_sql_error_handler($query, ibase_errmsg(), ibase_errcode(), db_version());
		return $r;
	}
} else {
	function q($query)
	{
		if (!isset($GLOBALS['__DB_INC_INTERNALS__']['query_count'])) {
			$GLOBALS['__DB_INC_INTERNALS__']['query_count'] = 1;
		} else {
			++$GLOBALS['__DB_INC_INTERNALS__']['query_count'];
		}
	
		if (!isset($GLOBALS['__DB_INC_INTERNALS__']['total_sql_time'])) {
			$GLOBALS['__DB_INC_INTERNALS__']['total_sql_time'] = 0;
		}
	
		$s = gettimeofday();
		$result = ibase_query($query) or fud_sql_error_handler($query, ibase_errmsg(), ibase_errcode(), db_version());
		$e = gettimeofday(); 

		$GLOBALS['__DB_INC_INTERNALS__']['last_time'] = ($e['sec'] - $s['sec'] + (($e['usec'] - $s['usec'])/1000000));
		$GLOBALS['__DB_INC_INTERNALS__']['total_sql_time'] += $GLOBALS['__DB_INC_INTERNALS__']['last_time'];
		$GLOBALS['__DB_INC_INTERNALS__']['last_query'] = $query;

		echo '<pre>'.preg_replace('!\s+!', ' ', $query).'</pre>';
		echo '<pre>query count: '.$GLOBALS['__DB_INC_INTERNALS__']['query_count'].' time taken: '.$GLOBALS['__DB_INC_INTERNALS__']['last_time'].'</pre>';
		echo '<pre>Affected rows: '.db_affected().'</pre>';
		echo '<pre>total sql time: '.$GLOBALS['__DB_INC_INTERNALS__']['total_sql_time'].'</pre>';

		return $result; 
	}

	function uq($query)
	{
		return q($query);
	}
}

function db_count($result)
{
	return (int) ibase_num_rows($result);
}

function db_rowobj($result)
{
	return ibase_fetch_object($result);
}

function db_rowarr($result)
{
	return ibase_fetch_row($result);
}

function q_singleval($query)
{
	if (($res = ibase_fetch_row(q($query))) !== false) {
		return $res[0];
	}
}

function q_limit($query, $limit, $off)
{
	return 'SELECT FIRST '. $limit . ((!empty($off)) ? ' SKIP ' . $off : '') . substr($query, 6);
}

function get_fud_table_list($tbl='')
{
	if ($tbl) {
		return q_singleval('SELECT rdb$relation_name FROM rdb$relations WHERE rdb$view_blr is null AND(rdb$system_flag is null or rdb$system_flag = 0) AND rdb$relation_name LIKE \''. $tbl .'\'');
	}

	$ret = array();
	$c = uq('SELECT rdb$relation_name FROM rdb$relations WHERE rdb$view_blr is null AND(rdb$system_flag is null or rdb$system_flag = 0) AND rdb$relation_name LIKE \''. $GLOBALS['DBHOST_TBL_PREFIX'] .'%\'');
	while ($r = db_rowarr($c)) {
		$ret[] = $r[0];
	}

	return $ret;	
}

function optimize_tables($tbl_list=null)
{
	if (!$tbl_list) {
		$tbl_list = get_fud_table_list();
	}

	q('OPTIMIZE TABLE '. implode(', ', $tbl_list));
}

function db_saq($q)
{
	return ibase_fetch_row(q($q));
}
function db_sab($q)
{
	return ibase_fetch_object(q($q));
}
function db_qid($q)
{
	q($q);
	return ibase_insert_id(fud_sql_lnk);
}
function db_arr_assoc($q)
{
	return ibase_fetch_array(q($q), ibase_ASSOC);
}

function db_fetch_array($q)
{
        return is_object($q) ? ibase_fetch_array($q) : null;
}

function db_li($q, &$ef, $li=0)
{
	$r = ibase_query($q);
	if ($r) {
		return ($li ? ibase_insert_id(fud_sql_lnk) : $r);
	}

	/* duplicate key */
	if (ibase_errcode() == 1062) {
		$ef = ltrim(strrchr(ibase_errmsg(), ' '));
		return null;
	} else {
		fud_sql_error_handler($q, ibase_errmsg(), ibase_errcode(), db_version());
	}
}

function ins_m($tbl, $flds, $vals, $type=0)
{
	q("INSERT IGNORE INTO ".$tbl." (".$flds.") VALUES (".implode('),(', $vals).")");
}

function db_all($q)
{
	$f = array();
	$c = uq($q);
	while ($r = ibase_fetch_row($c)) {
		$f[] = $r[0];
	}
	return $f;
}

function _esc($s)
{
	return "'".ibase_real_escape_string($s, fud_sql_lnk)."'";
}
?>