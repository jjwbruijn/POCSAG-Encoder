<?php

require_once("autoload.php");

$enc = new encoder("10.0.0.22", "8080");
$syslog = new syslog_receiver("10.0.8.10",514);

while(1){
	$msg = $syslog->GetMsg();
	if($msg->severity<3){
		$message = $msg->daemon."@".$msg->hostname."\n".$msg->message;
		$enc->SendPage(pager::EmergencyPager(), $message);
		echo "------------Sent: $message\n\n";
	}
}
