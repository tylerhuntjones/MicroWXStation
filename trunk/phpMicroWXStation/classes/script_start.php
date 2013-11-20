<?
/*
 *
 * phpImgDump (php5-apache) Version 0.9.x Beta
 * Copyright (C) 2011  Tyler H. Jones (me@tylerjones.me)
 * http://www.thebasementserver.com/phpimgdump
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

/*-- Script Start Class --------------------------------*/
/*------------------------------------------------------*/
/* This isnt really a class but a way to tie other	*/
/* classes and functions used all over the site to the  */
/* page currently being displayed.			*/
/*------------------------------------------------------*/
/* The code that includes the main php files and	*/
/* generates the page are at the bottom.		*/
/*------------------------------------------------------*/
/********************************************************/

require 'config.php'; //The config contains all site wide configuration information
error_reporting(E_ALL); ini_set('display_errors', true); //Debug stuff

if(DEBUG_MODE) { error_reporting(E_ALL); ini_set('display_errors', true); }

$ScriptStartTime=microtime(true); //To track how long a page takes to create

session_start();
ob_start(); //Start a buffer, mainly in case there is a mysql error

//require(SERVER_ROOT.'classes/class_debug.php'); //Require the debug class
require(SERVER_ROOT.'classes/class_mysql.php'); //Require the database wrapper
//require(SERVER_ROOT.'classes/class_time.php'); //Require the time class

//$Debug = new DEBUG;
//$Debug->handle_errors();
$DB = new DB_MYSQL;

$BeginErrorBox = '<div id="error_bar" class="error_bar">';
$EndErrorBox = '</div>';


/**********************************************************************/
/** Check Config and Paths
************************************************************************/

// CH-CH-Check Upload c$_FILES["file"]["name"]onfig

//$Cache->flush();

// This function is to include the header file on a page.
// $JSIncludes is a comma separated list of js files to be inclides on
// the page, ONLY PUT THE RELATIVE LOCATION WITHOUT .js
// ex: 'somefile,somdire/somefile'

/*-- show_footer function ------------------------------------------------*/
/*------------------------------------------------------------------------*/
/**************************************************************************/

function check_var($var) {
	if(!isset($var) && empty($var)) {
		return false;
	} else {
		return true;
	}
}

function str_contains($substring, $string) {
        $pos = strpos($string, $substring);
        if($pos === false) { return false; } else { return true; }
}

function is_number($Str) {
	$Return = true;
	if ($Str < 0) { $Return = false; }
	// We're converting input to a int, then string and comparing to original
	$Return = ($Str == strval(intval($Str)) ? true : false);
	return $Return;
}

function CtoF($t) {
	return ($t * 1.8) + 32;
}

function getDirString($d) {
	if($d < 0) { return "Falling"; }
	if($d == 0) { return "Holding"; }
	if($d > 0) { return "Rising"; }
	return "Holding";
}

//Finds diff beetween a time and the current time, past or future.
function dateDiff($time1, $time2="now", $precision = 4) {
	if ($time2 == "now") { $time2 = time(); }
    	if (!is_numeric($time1)) { $time1 = strtotime($time1); }
    	if (!is_numeric($time2)) { $time2 = strtotime($time2); }
    	//$time2 = time();

	$suffix = " ago";
    	if ($time1 > $time2) {
    		$ttime = $time1;
      		$time1 = $time2;
      		$time2 = $ttime;
		$suffix = " from now";
    	}
 
	$tt = 0;
	if($time1 > $time2) { $tt = $time1-$time2; } else { $tt = $time2-$time1; }
	if($tt < 60) { return (string)$tt." seconds".$suffix; }
	elseif($tt < 3600 && $tt > 60) { return (string)(int)($tt/60)." minutes".$suffix; }
	elseif($tt > 3600 && $tt < 86400) { return (string)(int)($tt/3600)." hours".$suffix; }

    	$intervals = array('year','month','day','hour','minute','second');
    	$diffs = array();
 
    	foreach ($intervals as $interval) {
      		$diffs[$interval] = 0;
      		$ttime = strtotime("+1 " . $interval, $time1);
      		while ($time2 >= $ttime) {
			$time1 = $ttime;
			$diffs[$interval]++;
			$ttime = strtotime("+1 " . $interval, $time1);
      		}
    	}
 
    	$count = 0;
    	$times = array();
    	foreach ($diffs as $interval => $value) {
      		if ($count >= $precision) { break; }
	      	if ($value > 0) {
			if ($value != 1) { $interval .= "s"; }
			$times[] = $value . " " . $interval;
			$count++;
	      	}
    	}
    return implode(", ", $times).$suffix;
}

// This is preferable to htmlspecialchars because it doesn't screw up upon a double escape
function display_str($Str) {
	if (empty($Str)) {
		return '';
	}
	if ($Str!='' && !is_number($Str)) {
		$Str=make_utf8($Str);
		$Str=mb_convert_encoding($Str,"HTML-ENTITIES","UTF-8");
		$Str=preg_replace("/&(?![A-Za-z]{0,4}\w{2,3};|#[0-9]{2,5};)/m","&amp;",$Str);

		$Replace = array(
			"'",'"',"<",">",
			'&#128;','&#130;','&#131;','&#132;','&#133;','&#134;','&#135;','&#136;','&#137;','&#138;','&#139;','&#140;','&#142;','&#145;','&#146;','&#147;','&#148;','&#149;','&#150;','&#151;','&#152;','&#153;','&#154;','&#155;','&#156;','&#158;','&#159;'
		);

		$With=array(
			'&#39;','&quot;','&lt;','&gt;',
			'&#8364;','&#8218;','&#402;','&#8222;','&#8230;','&#8224;','&#8225;','&#710;','&#8240;','&#352;','&#8249;','&#338;','&#381;','&#8216;','&#8217;','&#8220;','&#8221;','&#8226;','&#8211;','&#8212;','&#732;','&#8482;','&#353;','&#8250;','&#339;','&#382;','&#376;'
		);

		$Str=str_replace($Replace,$With,$Str);
	}
	return $Str;
}

function make_utf8($Str) {
	if ($Str!="") {
		if (is_utf8($Str)) { $Encoding="UTF-8"; }
		if (empty($Encoding)) { $Encoding=mb_detect_encoding($Str,'UTF-8, ISO-8859-1'); }
		if (empty($Encoding)) { $Encoding="ISO-8859-1"; }
		if ($Encoding=="UTF-8") { return $Str; }
		else { return @mb_convert_encoding($Str,"UTF-8",$Encoding); }
	}
}

function is_utf8($Str) {
	return preg_match('%^(?:
		[\x09\x0A\x0D\x20-\x7E]			 // ASCII
		| [\xC2-\xDF][\x80-\xBF]			// non-overlong 2-byte
		| \xE0[\xA0-\xBF][\x80-\xBF]		// excluding overlongs
		| [\xE1-\xEC\xEE\xEF][\x80-\xBF]{2} // straight 3-byte
		| \xED[\x80-\x9F][\x80-\xBF]		// excluding surrogates
		| \xF0[\x90-\xBF][\x80-\xBF]{2}	 // planes 1-3
		| [\xF1-\xF3][\x80-\xBF]{3}		 // planes 4-15
		| \xF4[\x80-\x8F][\x80-\xBF]{2}	 // plane 16
		)*$%xs', $Str
	);
}

// Escape an entire array for output
// $Escape is either true, false, or a list of array keys to not escape
function display_array($Array, $Escape = array()) {
	foreach ($Array as $Key => $Val) {
		if((!is_array($Escape) && $Escape == true) || !in_array($Key, $Escape)) {
			$Array[$Key] = display_str($Val);
		}
	}
	return $Array;
}

// Generate a random (secret) strings
function make_secret($Length = 32) { //Make a secret key with lower case letters and numbers only
	$Secret = '';
	$Chars='abcdefghijklmnopqrstuvwxyz0123456789';
	for($i=0; $i<$Length; $i++) {
		$Rand = mt_rand(0, strlen($Chars)-1);
		$Secret .= substr($Chars, $Rand, 1);
	}
	return str_shuffle($Secret);
}

function make_complex_secret($Length = 32) { //Make a secret key with upper and lower case letters and numbers
	$Secret = '';
	$Chars='ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
	for($i=0; $i<$Length; $i++) {
		$Rand = mt_rand(0, strlen($Chars)-1);
		$Secret .= substr($Chars, $Rand, 1);
	}
	return str_shuffle($Secret);
}

// Password hashes, feel free to make your own algorithm here
function make_hash($Str,$Secret=SITE_SALT) {
	return sha1(md5($Secret).$Str.sha1($Secret).SITE_SALT);
}

function error($Error, $Ajax=false) {
	//global $Debug;
 	//require(SERVER_ROOT.'sections/error/index.php');
	//$Debug->profile();
 	die();
}

/**
 * @param BanReason 0 - Unknown, 1 - Manual, 2 - Ratio, 3 - Inactive, 4 - Unused.
 */

/** This ends_with is slightly slower when the string is found, but a lot faster when it isn't.
 */
function ends_with($Haystack, $Needle) {
	return substr($Haystack, strlen($Needle) * -1) == $Needle;
}

function starts_with($Haystack, $Needle) {
	return strpos($Haystack, $Needle) === 0;
}

//Include /sections/*/index.php
//$Document = basename(parse_url($_SERVER['SCRIPT_FILENAME'], PHP_URL_PATH), '.php');
//if(!preg_match('/^[a-z0-9]+$/i', $Document)) { error(404); }
//require(SERVER_ROOT.'sections/'.$Document.'/index.php');


//Flush to user
ob_end_flush();
