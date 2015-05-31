<?php

error_reporting(E_ALL);
ini_set('display_errors', true);
require('classes/script_start.php'); 

global $DB;

class MinStruct {
	public $temp = 9999;
	public $hi = 9999;
	public $pres = 9999;
	public $humid = 9999;
	public $dp = 9999;
}

class MaxStruct {
	public $temp = -100;
	public $hi = -100;
	public $pres = -100;
	public $humid = -100;
	public $dp = -100;
}

$min = new MinStruct();
$max = new MaxStruct();
$time = 24;

if(isset($_GET['celcius'])) {
	$usec = true;
} else {
	$usec = false;
}

if(isset($_GET['time'])) {
	if(is_number($_GET['time'])) {
		$time = $_GET['time'];
	}
}

$DB->query("SELECT time FROM data ORDER BY time DESC LIMIT 1");
if($DB->record_count() < 1) { 
	echo "No data returned from DB!";
	die();
} else {
	list($latest_timestamp) = $DB->next_record();
}

$limittime = $latest_timestamp - ($time * 3600);

$DB->query("SELECT tempf, humidity, pressure, dewpoint, heatindex FROM data WHERE time>=$limittime");
if($DB->record_count() < 1) { 
	echo "No data returned from DB!";
	die();
} else {
	$temp_array = $DB->collect("tempf");
	$heatindex_array = $DB->collect("heatindex");
	$humidity_array = $DB->collect("humidity");
	$pressure_array = $DB->collect("pressure");
	$dewpoint_array = $DB->collect("dewpoint");
}

for($i = 0; $i < $DB->record_count(); $i++) {
	if($temp_array[$i] > $max->temp) { $max->temp = $temp_array[$i]; }
	if($heatindex_array[$i] > $max->hi) { $max->hi = $heatindex_array[$i]; }
	if($humidity_array[$i] > $max->humid) { $max->humid = $humidity_array[$i]; }
	if($pressure_array[$i] > $max->pres) { $max->pres = $pressure_array[$i]; }
	if($dewpoint_array[$i] > $max->dp) { $max->dp = $dewpoint_array[$i]; }

	if($temp_array[$i] < $min->temp) { $min->temp = $temp_array[$i]; }
	if($heatindex_array[$i] < $min->hi) { $min->hi = $heatindex_array[$i]; }
	if($humidity_array[$i] < $min->humid) { $min->humid = $humidity_array[$i]; }
	if($pressure_array[$i] < $min->pres) { $min->pres = $pressure_array[$i]; }
	if($dewpoint_array[$i] < $min->dp) { $min->dp = $dewpoint_array[$i]; }
}

?>
<html>
<body>
<?
if($usec) {
?>
<a href="minmax.php?time=<?=$time?>">Use Farenheit</a><br />
<? } else { ?>
<a href="minmax.php?time=<?=$time?>&celcius">Use Celcius</a><br />
<? } ?>
<table border="5" width="30%" cellpadding="2" cellspacing="2">
   <tr>
      <th colspan="3"><br><h3>Min/Max Values (Last <?=$time?>Hrs)</h3>
      </th>
   </tr>
   <tr>
      <td align="right"><b>Temperature</b></td>
      <td align="left"><?=$usec?FtoC($min->temp):$min->temp?> <?=$usec?'C':'F'?></td>
      <td align="left"><?=$usec?FtoC($max->temp):$max->temp?> <?=$usec?'C':'F'?></td>
   </tr>
   <tr>
      <td align="right"><b>Humidity</b></td>
      <td align="left"><?=$min->humid?>%</td>
      <td align="left"><?=$max->humid?>%</td>
   </tr>
   <tr>
      <td align="right"><b>Pressure</b></td>
      <td align="left"><?=$min->pres?>mb</td>
      <td align="left"><?=$max->pres?>mb</td>
   </tr>
   <tr>
      <td align="right"><b>Dew Point</b></td>
      <td align="left"><?=$usec?FtoC($min->dp):$min->dp?> <?=$usec?'C':'F'?></td>
      <td align="left"><?=$usec?FtoC($max->dp):$max->dp?> <?=$usec?'C':'F'?></td>
   </tr>
   <tr>
      <td align="right"><b>Heat Index</b></td>
      <td align="left"><?=$usec?FtoC($min->hi):$min->hi?> <?=$usec?'C':'F'?></td>
      <td align="left"><?=$usec?FtoC($max->hi):$max->hi?> <?=$usec?'C':'F'?></td>
   </tr>
</table>
<br />
<hr>
<br />
Time Span -- <a href="minmax.php?time=168">1 Week</a> | <a href="minmax.php?time=48">48Hrs</a> | <a href="minmax.php?time=24">24Hrs</a>

</body>
</html>

