<?php

class syslog_receiver{
	protected $sock;
	public function __construct($ip,$port){
		if(!($this->sock = socket_create(AF_INET, SOCK_DGRAM, 0))){
			$errorcode = socket_last_error();
			$errormsg = socket_strerror($errorcode);
			die("Couldn't create socket: [$errorcode] $errormsg \n");
		}
		
		if(!socket_bind($this->sock, $ip , $port)){
			$errorcode = socket_last_error();
			$errormsg = socket_strerror($errorcode);
			die("Could not bind socket : [$errorcode] $errormsg \n");
		}
	}
	
	public function GetMsg(){
		socket_recvfrom($this->sock, $buf, 2048, 0, $ip, $port);
		return syslog_message::Get($buf);
	}
	
}

class syslog_message{
	public $facility = "";
	public $severity = "";
	public $date = "";
	public $hostname = "";
	public $daemon = "";
	public $daemonPID = "";
	public $message = "";
	
	public function __construct($message=""){
		list($priority,$message) = explode(">",$message, 2);
		$priority = substr($priority,1);
		$this->facility = floor($priority/8);
		$this->severity = $priority%8;
		$this->date = substr($message,0,15);
		$message = substr($message,16);
		list($this->hostname,$message) = explode(" ",$message,2);
		$this->hostname = trim(strtolower($this->hostname));
		list($daemon,$message) = explode(":",$message,2);
		if((strstr($daemon,"["))&&(strstr($daemon,"]"))){
			list($daemon,$pid)=explode("[",$daemon);
			$this->daemonPID=trim(substr($pid,0,-1));
			$this->daemon = trim($daemon);
		} else {
			$this->daemon = trim($daemon);
		}
		$this->daemon = trim($this->daemon);
		$this->daemonPID = trim($this->daemonPID);
		$this->message = trim($message);
	}
	
	public static function Get($message){
		$msg = new syslog_message($message);
		return $msg;
	}
	
}