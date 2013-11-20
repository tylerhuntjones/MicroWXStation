<?

error_reporting(E_ALL);
ini_set('display_errors', true);
require('classes/script_start.php'); 

global $DB;

class MinStruct {
	public $dht = 9999;
	public $bmp = 9999;
	public $pres = 9999;
	public $humid = 9999;
	public $dp = 9999;
}

class MaxStruct {
	public $dht = -100;
	public $bmp = -100;
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

$DB->query("SELECT timestamp FROM log ORDER BY timestamp DESC LIMIT 1");
if($DB->record_count() < 1) { 
	echo "No data returned from DB!";
	die();
} else {
	list($latest_timestamp) = $DB->next_record();
}

$limittime = $latest_timestamp - ($time * 3600);

$DB->query("SELECT dht_temp, bmp_temp, humidity, pressure, dewpoint FROM log WHERE timestamp>=$limittime");
if($DB->record_count() < 1) { 
	echo "No data returned from DB!";
	die();
} else {
	$dht_temp_array = $DB->collect("dht_temp");
	$bmp_temp_array = $DB->collect("bmp_temp");
	$humidity_array = $DB->collect("humidity");
	$pressure_array = $DB->collect("pressure");
	$dewpoint_array = $DB->collect("dewpoint");
}

for($i = 0; $i < $DB->record_count(); $i++) {
	if($dht_temp_array[$i] > $max->dht) { $max->dht = $dht_temp_array[$i]; }
	if($bmp_temp_array[$i] > $max->bmp) { $max->bmp = $bmp_temp_array[$i]; }
	if($humidity_array[$i] > $max->humid) { $max->humid = $humidity_array[$i]; }
	if($pressure_array[$i] > $max->pres) { $max->pres = $pressure_array[$i]; }
	if($dewpoint_array[$i] > $max->dp) { $max->dp = $dewpoint_array[$i]; }

	if($dht_temp_array[$i] < $min->dht) { $min->dht = $dht_temp_array[$i]; }
	if($bmp_temp_array[$i] < $min->bmp) { $min->bmp = $bmp_temp_array[$i]; }
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
      <td align="right"><b>Temp 1</b></td>
      <td align="left"><?=$usec?$min->dht:CtoF($min->dht)?> <?=$usec?'C':'F'?></td>
      <td align="left"><?=$usec?$max->dht:CtoF($max->dht)?> <?=$usec?'C':'F'?></td>
   </tr>
   <tr>
      <td align="right"><b>Temp 2</b></td>
      <td align="left"><?=$usec?$min->bmp:CtoF($min->bmp)?> <?=$usec?'C':'F'?></td>
      <td align="left"><?=$usec?$max->bmp:CtoF($max->bmp)?> <?=$usec?'C':'F'?></td>
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
      <td align="left"><?=$usec?$min->dp:CtoF($min->dp)?> <?=$usec?'C':'F'?></td>
      <td align="left"><?=$usec?$max->dp:CtoF($max->dp)?> <?=$usec?'C':'F'?></td>
   </tr>
</table>
<br />
<hr>
<br />
Time Span -- <a href="minmax.php?time=168">1 Week</a> | <a href="minmax.php?time=48">48Hrs</a> | <a href="minmax.php?time=24">24Hrs</a>

</body>
</html>

