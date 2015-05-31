<?php   

error_reporting(E_ALL);
ini_set('display_errors', true);
require('classes/script_start.php'); 
include("classes/pchart/pData.class.php");
include("classes/pchart/pDraw.class.php");
include("classes/pchart/pImage.class.php");

global $DB;

if(isset($_GET['span'])) {
	if($_GET['span'] == "24") {
		$span = 24;
	} elseif($_GET['span'] == "12") {
		$span = 12;
	} elseif($_GET['span'] == "6") {
		$span = 6;
	} elseif($_GET['span'] == "1") {
		$span = 1;
	} else {
		$span = 1;
	}
} else {
	$span = 24;
}

$DB->query("SELECT time FROM data ORDER BY time DESC LIMIT 1");
if($DB->record_count() < 1) { 
	echo "No data returned from DB!";
	die();
} else {
	list($latest_timestamp) = $DB->next_record();
}

if($span==24) {
	$DB->query("SELECT time, pressure FROM data WHERE time>=($latest_timestamp-86400) ORDER BY time ASC");
	$raw_pressure_array = $DB->collect("pressure");
	$raw_times_array = $DB->collect("time");
	$pressure_array = array();
	$times_array = array();
	for($i = 24; $i > 1; $i--) {
		$t = $latest_timestamp - ($i * 3600);
		for($j = 0; $j < count($raw_pressure_array); $j++) {
			if($raw_times_array[$j] > $t) {
				$pressure_array[24-$i] = $raw_pressure_array[$j];
				$times_array[24-$i] = $raw_times_array[$j];
				$j = count($raw_pressure_array) + 1;
				//continue;
			}
		}
	}
	for($i = 0; $i < count($times_array); $i++) {
		$times_array[$i] = gmdate("H:i", $times_array[$i]-18000);
	}
} elseif($span==12) {
	$DB->query("SELECT time, pressure FROM data WHERE time>=$latest_timestamp-43200 ORDER BY time ASC");
	$raw_pressure_array = $DB->collect("pressure");
	$raw_times_array = $DB->collect("time");
	$pressure_array = array();
	$times_array = array();
	for($i = 24; $i > 1; $i--) {
		$t = $latest_timestamp - ($i * 1800);
		for($j = 0; $j < count($raw_pressure_array); $j++) {
			if($raw_times_array[$j] > $t) {
				$pressure_array[24-$i] = $raw_pressure_array[$j];
				$times_array[24-$i] = $raw_times_array[$j];
				$j = count($raw_pressure_array) + 1;
				//continue;
			}
		}
	}
	for($i = 0; $i < count($times_array); $i++) {
		$times_array[$i] = gmdate("H:i", $times_array[$i]-18000);
	}
} elseif($span==6) {
	$DB->query("SELECT time, pressure FROM data WHERE time>=$latest_timestamp-21600 ORDER BY time ASC");
	$raw_pressure_array = $DB->collect("pressure");
	$raw_times_array = $DB->collect("time");
	$pressure_array = array();
	$times_array = array();
	for($i = 24; $i > 1; $i--) {
		$t = $latest_timestamp - ($i * 900);
		for($j = 0; $j < count($raw_pressure_array); $j++) {
			if($raw_times_array[$j] > $t) {
				$pressure_array[24-$i] = $raw_pressure_array[$j];
				$times_array[24-$i] = $raw_times_array[$j];
				$j = count($raw_pressure_array) + 1;
				//continue;
			}
		}
	}
	for($i = 0; $i < count($times_array); $i++) {
		$times_array[$i] = gmdate("H:i", $times_array[$i]-18000);
	}
} elseif($span==1) {
	$DB->query("SELECT time, pressure FROM data WHERE time>=$latest_timestamp-3600 ORDER BY time ASC");
	$raw_pressure_array = $DB->collect("pressure");
	$raw_times_array = $DB->collect("time");
	$pressure_array = array();
	$times_array = array();
	for($i = 24; $i > 1; $i--) {
		$t = $latest_timestamp - ($i * 150);
		for($j = 0; $j < count($raw_pressure_array); $j++) {
			if($raw_times_array[$j] > $t) {
				$pressure_array[24-$i] = $raw_pressure_array[$j];
				$times_array[24-$i] = $raw_times_array[$j];
				$j = count($raw_pressure_array) + 1;
				//continue;
			}
		}
	}
	for($i = 0; $i < count($times_array); $i++) {
		$times_array[$i] = gmdate("H:i", $times_array[$i]-18000);
	}
}

 /* Create and populate the pData object */
 $MyData = new pData();  
 $MyData->addPoints($pressure_array,"Pressure");
 $MyData->setAxisName(0,"Millibars");
 $MyData->addPoints($times_array,"Labels");
 $MyData->setSerieDescription("Labels");
 $MyData->setAbscissa("Labels");

 /* Create the pChart object */
 $myPicture = new pImage(1000,730,$MyData);

 
 /* Write the picture title */ 
 $myPicture->setFontProperties(array("FontName"=>"fonts/Silkscreen.ttf","FontSize"=>6));
 $myPicture->drawText(10,13,"drawPlotChart() - draw a plot chart",array("R"=>255,"G"=>255,"B"=>255));

 /* Write the chart title */ 
 $myPicture->setFontProperties(array("FontName"=>"fonts/Forgotte.ttf","FontSize"=>11));
 $myPicture->drawText(250,55,"Pressure",array("FontSize"=>20,"Align"=>TEXT_ALIGN_BOTTOMMIDDLE));

 /* Draw the scale and the 1st chart */
 $myPicture->setGraphArea(60,60,950,390);
 $myPicture->drawFilledRectangle(60,60,950,390,array("R"=>255,"G"=>255,"B"=>255,"Surrounding"=>-200,"Alpha"=>10));
 $myPicture->drawScale(array("DrawSubTicks"=>TRUE));
 $myPicture->setShadow(TRUE,array("X"=>1,"Y"=>1,"R"=>0,"G"=>0,"B"=>0,"Alpha"=>10));
 $myPicture->setFontProperties(array("FontName"=>"fonts/pf_arma_five.ttf","FontSize"=>6));
 $myPicture->drawLineChart(array("DisplayValues"=>TRUE,"DisplayColor"=>DISPLAY_AUTO));
 $myPicture->setShadow(FALSE);

 /* Write the chart legend */
 //$myPicture->drawLegend(510,205,array("Style"=>LEGEND_NOBORDER,"Mode"=>LEGEND_HORIZONTAL));

 /* Render the picture (choose the best way) */
 $myPicture->autoOutput("pictures/example.drawLineChart.png");
?>
