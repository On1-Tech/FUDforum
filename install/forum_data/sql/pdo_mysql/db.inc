<?php
/**
* copyright            : (C) 2001-2009 Advanced Internet Designs Inc.
* email                : forum@prohost.org
* $Id: db.inc,v 1.1 2009/08/06 17:43:45 frank Exp $
*
* This program is free software; you can redistribute it and/or modify it 
* under the terms of the GNU General Public License as published by the 
* Free Software Foundation; version 2 of the License. 
**/
# define('forum_debug', 1);

class db { public static $db, $ar, $res, $buf; }

if (empty(db::$db)) {
	if ($GLOBALS['DBHOST']{0} == ':') {
		$host = 'unix_socket='.substr($GLOBALS['DBHOST'], 1);
	} else {
		$host = 'host='.$GLOBALS['DBHOST'];
	}

	$dsn = 'mysql:'.$host.';dbname='.$GLOBALS['DBHOST_DBNAME'];
	define('__FUD_SQL_CONCAT__', 'CONCAT');

	try {
		db::$db = new PDO($dsn, $GLOBALS['DBHOST_USER'], $GLOBALS['DBHOST_PASSWORD']);
	} catch (PDOException $e) {
		fud_sql_error_handler("Failed to establish database connection, PDO says: ".$e->getMessage(), '', '', '');
	}

	db::$db->exec("SET NAMES 'utf8' COLLATE 'utf8_general_ci");

	define('__dbtype__', substr($GLOBALS['DBHOST_DBTYPE'], 4));
}

function get_version()
{
	if (!defined('__FUD_SQL_VERSION__')) {
		define('__FUD_SQL_VERSION__', db::$db->getAttribute(PDO::ATTR_SERVER_VERSION));
	}
	return __FUD_SQL_VERSION__;
}

function db_lock($tables)
{
	if (!empty($GLOBALS['__DB_INC_INTERNALS__']['db_locked'])) {
		fud_sql_error_handler("Recursive Lock", "internal", "internal", get_version());
	}
	q('LOCK TABLES '.$tables);
	$GLOBALS['__DB_INC_INTERNALS__']['db_locked'] = 1;
}

function db_unlock()
{
	if (empty($GLOBALS['__DB_INC_INTERNALS__']['db_locked'])) {
		unset($GLOBALS['__DB_INC_INTERNALS__']['db_locked']);
		fud_sql_error_handler("DB_UNLOCK: no previous lock established", "internal", "internal", get_version());
	}
	
	if (--$GLOBALS['__DB_INC_INTERNALS__']['db_locked'] < 0) {
		unset($GLOBALS['__DB_INC_INTERNALS__']['db_locked']);
		fud_sql_error_handler("DB_UNLOCK: unlock overcalled", "internal", "internal", get_version());
	}

	q('UNLOCK TABLES');

	unset($GLOBALS['__DB_INC_INTERNALS__']['db_locked']);
}

function db_locked()
{
	return isset($GLOBALS['__DB_INC_INTERNALS__']['db_locked']);
}

function db_affected()
{
	return db::$ar;
}

function __enifo($a)
{
	return end($a);
}

function uq($query, $buf=0)
{
	if (!strncasecmp($query, 'SELECT', 6)) {
		db::$res = null;
		db::$buf = $buf;
		if ($buf && __dbtype__ == 'mysql') db::$db->setAttribute(PDO::MYSQL_ATTR_USE_BUFFERED_QUERY, 1);
		db::$res = db::$db->query($query) or fud_sql_error_handler($query, db::$db->errorCode(), __enifo(db::$db->errorInfo()), get_version());
		if ($buf && __dbtype__ == 'mysql') db::$db->setAttribute(PDO::MYSQL_ATTR_USE_BUFFERED_QUERY, 0);
		return db::$res;
	}
	if (!db::$buf) {
		db::$res = null;
	}
	if ((db::$ar = db::$db->exec($query)) === FALSE) {
		fud_sql_error_handler($query, db::$db->errorCode(), __enifo(db::$db->errorInfo()), get_version());
	}
	return db::$ar;
}

function q($query)
{
	return uq($query, 1);
}

function db_count($result)
{
	return (int) $result->rowCount();
}

function db_rowobj($result)
{
	return $result->fetch(PDO::FETCH_OBJ);
}

function db_rowarr($result)
{
	return $result->fetch(PDO::FETCH_NUM);
}

function q_singleval($query)
{
	return uq($query)->fetchColumn();
}

function qry_limit($limit, $off)
{
	return $off.','.$limit;
}

function get_fud_table_list($tbl='')
{
	if (!$tbl) {
		$idx = str_replace("_", "\\_", $GLOBALS['DBHOST_TBL_PREFIX']);
	}

	db::$buf = db::$res = null;

	if (!$tbl) {
		return db::$db->query("SHOW TABLES LIKE '".$idx."%'")->fetchAll(PDO::FETCH_COLUMN);
	}
	return db::$db->query("SHOW TABLES LIKE '".str_replace("_", "\\_", $tbl)."'")->fetchColumn();
}

function optimize_tables($tbl_list=null)
{
	if (!$tbl_list) {
		$tbl_list = get_fud_table_list();
	}

	$res = db::$db->query('OPTIMIZE TABLE '. implode(', ', $tbl_list));
	unset($res);
}

function db_saq($q)
{
	return uq($q)->fetch(PDO::FETCH_NUM);
}

function db_sab($q)
{
	return uq($q)->fetch(PDO::FETCH_OBJ);
}

function db_qid($q)
{
	uq($q);
	return db::$db->lastInsertId();
}

function db_arr_assoc($q)
{
	return uq($q)->fetch(PDO::FETCH_ASSOC);
}

function db_fetch_array($q)
{
	return $q->fetch(PDO::FETCH_ASSOC);
}

function db_li($q, &$ef, $li=0)
{
	db::$buf = db::$res = null;
	$r = db::$db->exec($q);

	if ($r !== false) {
		if (!$li) {
			return $r;
		}
		return db::$db->lastInsertId();
	}

	/* duplicate key */
	if (($c = db::$db->errorCode()) == '23000' || $c == '23505') {
		$ef = ltrim(strrchr(__enifo(db::$db->errorInfo()), ' '));
		return null;
	} else {
		fud_sql_error_handler($q, db::$db->errorCode(), __enifo(db::$db->errorInfo()), get_version());
	}
}

function ins_m($tbl, $flds, $vals, $type=0, $strict=1)
{
	return uq("INSERT IGNORE INTO ".$tbl." (".$flds.") VALUES (".implode('),(', $vals).")");
}

function db_all($q)
{
	db::$buf = db::$res = null;
	return db::$db->query($q)->fetchAll(PDO::FETCH_COLUMN);
}

function _esc($s)
{
	return db::$db->quote($s);
}
?>