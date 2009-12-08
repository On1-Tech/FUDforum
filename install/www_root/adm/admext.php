<?php
/**
* copyright            : (C) 2001-2009 Advanced Internet Designs Inc.
* email                : forum@prohost.org
* $Id$
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; version 2 of the License.
**/

	require('./GLOBALS.php');
	fud_use('adm.inc', true);
	fud_use('ext.inc', true);
	
	include($WWW_ROOT_DISK . 'adm/header.php');	
	$tbl = $GLOBALS['DBHOST_TBL_PREFIX'];

	if (!empty($_POST['c_ext'])) {
		if (($p = strrpos($_POST['c_ext'], '.')) !== false) {
			$c_ext = rtrim(substr($_POST['c_ext'], ($p + 1)));
		} else {
			$c_ext = trim($_POST['c_ext']);
		}
	}

	if (isset($_POST['edit'], $_POST['btn_update']) && $c_ext) {
		q('UPDATE '.$tbl.'ext_block SET ext='._esc($c_ext).' WHERE id='.(int)$_POST['edit']);
		echo successify('File extention (*.'.$c_ext.') was successfully updated.');
	} else if (isset($_POST['btn_submit']) && $c_ext) {
		q('INSERT INTO '.$tbl.'ext_block (ext) VALUES('._esc($c_ext).')');
		echo successify('File extention (*.'.$c_ext.') was successfully added.');
	} else if (isset($_GET['del'])) {
		q('DELETE FROM '.$tbl.'ext_block WHERE id='.(int)$_GET['del']);
		echo successify('File extention was successfully removed.');
	} else {
		$nada = 1;
	}

	if (!isset($nada) && db_affected()) {
		ext_cache_rebuild();
		echo successify('Extension cache was successfully rebuilt.');
	}

	if (isset($_GET['edit'])) {
		list($edit, $c_ext) = db_saq('SELECT id, ext FROM '.$tbl.'ext_block WHERE id='.(int)$_GET['edit']);
	} else {
		$edit = $c_ext = '';
	}
?>
<h2>Allowed Extensions</h2>
<p>Specify which files types users can attach to forum messages. If no file extension is entered, all files will be allowed.</p>
<form id="exf" method="post" action="admext.php">
<table class="datatable solidtable">
	<tr class="field">
		<td>Allow File Extension:</td>
		<td><input tabindex="1" type="text" name="c_ext" value="<?php echo htmlspecialchars($c_ext); ?>" /></td>
	</tr>

	<tr class="fieldaction">
		<td colspan="2" align="right">
		<?php
			if ($edit) {
				echo '<input type="submit" name="btn_cancel" value="Cancel" /> <input type="submit" name="btn_update" value="Update" tabindex="2" />';
			} else {
				echo '<input tabindex="2" type="submit" name="btn_submit" value="Add" />';
			}
		?>
		</td>
	</tr>
</table>
<input type="hidden" name="edit" value="<?php echo $edit; ?>" />
<?php echo _hs; ?>
</form>
<script type="text/javascript">
/* <![CDATA[ */
document.forms['exf'].c_ext.focus();
/* ]]> */
</script>
<table class="resulttable fulltable">
<tr class="resulttopic">
	<td>Extension</td>
	<td>Action</td>
</tr>
<?php
	$c = uq('SELECT ext,id FROM '.$tbl.'ext_block');
	$i = 1;
	while ($r = db_rowarr($c)) {
		if ($edit == $r[0]) {
			$bgcolor = ' class="resultrow1"';
		} else {
			$bgcolor = ($i++%2) ? ' class="resultrow2"' : ' class="resultrow1"';
		}
		echo '<tr '.$bgcolor.'><td>'.htmlspecialchars($r[0]).'</td><td>[<a href="admext.php?edit='.$r[1].'&amp;'.__adm_rsid.'">Edit</a>] [<a href="admext.php?del='.$r[1].'&amp;'.__adm_rsid.'">Delete</a>]</td></tr>';
	}
	unset($c);
?>
</table>
<?php require($WWW_ROOT_DISK . 'adm/footer.php'); ?>
