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
?>

<?php 
error_reporting(E_ALL);
ini_set('display_errors', true);
require('classes/script_start.php'); 

global $DB;

if(isset($_GET['key'])) {
	if($_GET['key'] != "38H4IW2p8nf3DRp1d") {
		die("Bad authentication!");
	}
} else {
	die("Bad authentication!");
}

$DB->query("INSERT INTO data (time, tempf, pressure, humidity, dewpoint, heatindex, light, wind_speed, wind_dir, wind_gust_speed, wind_gust_dir, wind_speed_avg2m, wind_dir_avg2m, wind_gust_speed_avg10m, wind_gust_dir_avg10m) VALUES ('".time()."', '".$_GET["tf"]."', '".$_GET["pres"]."', '".$_GET["humid"]."', '".$_GET["dp"]."', '".$_GET["hi"]."', '".$_GET["lt"]."', '".$_GET["ws"]."', '".$_GET["wd"]."', '".$_GET["wgs"]."', '".$_GET["wgd"]."', '".$_GET["wsavg2m"]."', '".$_GET["wdavg2m"]."', '".$_GET["wgsavg10m"]."', '".$_GET["wgdavg10m"]."')");

// log.php?key=38H4IW2p8nf3DRp1d&tf=79.4&pres=1007.81&humid=55.8&dp=72.3&hi=89.5&lt=0.72&ws=8.77&wd=248&wgs=15.54&wgd=217&wsavg2m=0&wdavg2m=0&wgsavg10m=0&wgdavg10m=0

?>

