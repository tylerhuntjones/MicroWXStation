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

<? 
error_reporting(E_ALL);
ini_set('display_errors', true);
require('classes/script_start.php'); 

global $DB;

if(isset($_GET['celcius'])) {
	$usec = true;
} else {
	$usec = false;
}


//Get last record in log table
$DB->query("SELECT timestamp, dht_temp, bmp_temp, humidity, pressure, dewpoint, altitude FROM log ORDER BY timestamp DESC LIMIT 1");
if($DB->record_count() < 1) { 
	echo "No data returned from DB!";
	die();
} else {
	list($timestamp, $dht_temp, $bmp_temp, $humidity, $pressure, $dewpoint, $altitude) = $DB->next_record();
}

//Determine if WX values are rising or falling
$num_greater = 0;
$num_fewer = 0;
$DB->query("SELECT dht_temp, bmp_temp, pressure, dewpoint FROM log ORDER BY timestamp DESC LIMIT 21");
//list($dht_temp_dir, $bmp_temp_dir, $perssure_dir, $dewpoint_dir) = $DB->next_record();
$dht_temp_array = $DB->collect("dht_temp");
$bmp_temp_array = $DB->collect("bmp_temp");
$pressure_array = $DB->collect("pressure");
$dewpoint_array = $DB->collect("dewpoint");
//DHT Temperature
for($i = 1; $i < 21; $i++) {
	if($dht_temp_array[$i] < $dht_temp) {
		$num_fewer++;
	}
	if($dht_temp_array[$i] > $dht_temp) {
		$num_greater++;
	}
}
if($num_greater == $num_fewer) {
	$dht_temp_dir = 0;
} else {
	if($num_greater > $num_fewer) {
		$dht_temp_dir = -1;
	}
	if($num_greater < $num_fewer) {
		$dht_temp_dir = 1;
	}
}
$num_fewer = 0;
$num_greater = 0;
//BMP Temperature
for($i = 1; $i < 21; $i++) {
	if($bmp_temp_array[$i] < $bmp_temp) {
		$num_fewer++;
	}
	if($bmp_temp_array[$i] > $bmp_temp) {
		$num_greater++;
	}
}
if($num_greater == $num_fewer) {
	$bmp_temp_dir = 0;
} else {
	if($num_greater > $num_fewer) {
		$bmp_temp_dir = -1;
	}
	if($num_greater < $num_fewer) {
		$bmp_temp_dir = 1;
	}
}
$num_fewer = 0;
$num_greater = 0;
//Pressure
for($i = 1; $i < 21; $i++) {
	if($pressure_array[$i] < $pressure) {
		$num_fewer++;
	}
	if($pressure_array[$i] > $pressure) {
		$num_greater++;
	}
}
if($num_greater == $num_fewer) {
	$pressure_dir = 0;
} else {
	if($num_greater > $num_fewer) {
		$pressure_dir = -1;
	}
	if($num_greater < $num_fewer) {
		$pressure_dir = 1;
	}
}
$num_fewer = 0;
$num_greater = 0;
//Dew Point
for($i = 1; $i < 21; $i++) {
	if($dewpoint_array[$i] < $dewpoint) {
		$num_fewer++;
	}
	if($dewpoint_array[$i] > $dewpoint) {
		$num_greater++;
	}
}
if($num_greater == $num_fewer) {
	$dewpoint_dir = 0;
} else {
	if($num_greater > $num_fewer) {
		$dewpoint_dir = -1;
	}
	if($num_greater < $num_fewer) {
		$dewpoint_dir = 1;
	}
}
//---------------------------------------------------

if($usec) {
?>
<a href="index.php">Use Farenheit</a><br />
<? } else { ?>
<a href="index.php?celcius">Use Celcius</a><br />
<? } ?>
<table border="5" width="30%" cellpadding="2" cellspacing="2">
   <tr>
      <th colspan="3"><br><h3>Current WX Data</h3>
      </th>
   </tr>
   <tr>
      <td align="right"><b>Time</b></td>
      <td align="left"><?=gmdate("m/d/Y H:i:s", $timestamp-18000)?></td>
      <td align="center">---</td>
   </tr>
   <tr>
      <td align="right"><b>Temp 1</b></td>
      <td align="left"><?=$usec?$dht_temp:CtoF($dht_temp)?> <?=$usec?'C':'F'?></td>
      <td align="center"><?=getDirString($dht_temp_dir)?></td>
   </tr>
   <tr>
      <td align="right"><b>Temp 2</b></td>
      <td align="left"><?=$usec?$bmp_temp:CtoF($bmp_temp)?> <?=$usec?'C':'F'?></td>
      <td align="center"><?=getDirString($bmp_temp_dir)?></td>
   </tr>
   <tr>
      <td align="right"><b>Humidity</b></td>
      <td align="left"><?=$humidity?>%</td>
      <td align="center">---</td>
   </tr>
   <tr>
      <td align="right"><b>Pressure</b></td>
      <td align="left"><?=$pressure?> mb</td>
      <td align="center"><?=getDirString($pressure_dir)?></td>
   </tr>
   <tr>
      <td align="right"><b>Dew Point</b></td>
      <td align="left"><?=$usec?$dewpoint:CtoF($dewpoint)?> <?=$usec?'C':'F'?></td>
      <td align="center"><?=getDirString($dewpoint_dir)?></td>
   </tr>
   <tr>
      <td align="right"><b>Altitude</b></td>
      <td align="left"><?=$altitude?> ft</td>
      <td align="center">---</td>
   </tr>
</table>
<br />

<br />
<hr />
<br />
Pressure Charts -- <a href="chart_pressure.php?span=24">24Hr</a> | <a href="chart_pressure.php?span=12">12Hr</a> | <a href="chart_pressure.php?span=6">6Hr</a> | <a href="chart_pressure.php?span=1">1Hr</a>
<br />
<hr>
<br />
<a href="minmax.php">Min Max Values</a>
