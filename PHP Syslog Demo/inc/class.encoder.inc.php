<?php

/**
 * Handles communication with the encoder.
 * Takes an IP Address and port number, and attempts to connect
 * @param string $server Encoder IP Address
 * @param int $port Port number
 */
class encoder{
	
	protected $socket;
	public $isConnected;
	
	protected $RIC = 11111111111;
	protected $baudRate;
	protected $frequency;
	protected $func = 99;
	protected $pagerType = 99;
	
	public function __construct($server,$port){
		$this->socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
		if(!$this->socket){
			$this->isConnected = false;
			return false;
		}
		$res = socket_connect($this->socket, $server, $port);
		if(!$res){
			$this->isConnected = false;
			return false;
		}
		$this->isConnected = true;
	}
	
	/**
	 * Sends a page.
	 * @param object $pager - The pager object
	 * @param string $message - The message to send
	 * @return bool true on completed
	 */
	public function SendPage($pager,$message){
		$this->SetFrequency($pager->frequency);
		$this->SetRIC($pager->RIC);
		$this->SetBaudRate($pager->baudRate);
		$this->SetPagerType($pager->pagerType);
		$this->SetPagerFunction($pager->func);
		return $this->SendMessage($message);
	}
	
	/**
	 * Sends a message directly to the encoder.
	 * @param string $msg The message that needs to be sent. Should probably end in an \n
	 * @return string|false The string sent back by the encoder, or false if not connected
	 */
	protected function EncoderSend($msg){
		if($this->isConnected){
			socket_send($this->socket, $msg, strlen($msg), null);
			$ret = socket_read($this->socket, 2048);
			return $ret;
		} else {
			return false;
		}
		
	}
	
	/**
	 * Sends the MSG: part of the page to the encoder.
	 * @param string $message The message that needs to be sent
	 * @return bool true on completed
	 */
	protected function SendMessage($message){
		$message = str_replace("\n", "\\n", $message);
		if($this->EncoderSend("MSG:$message\r\n")){
			return true;
		} else {
			return false;
		}
	}
	
	/**
	 * Sets the frequency for paging on the encoder.
	 * @param float The frequency in MHz for transmit
	 */
	protected function SetFrequency($frequency){
		if($this->frequency!=$frequency){
			$this->frequency = $frequency;
			$hexvalue = $this->FreqToSI4432Value($frequency);
			$this->EncoderSend("FREQ:$hexvalue\r\n");
		}
	}
	
	/**
	 * Sets the baudrate for transmitting. Should be something like 1200 or 2400.
	 * @param int $baudrate the baudrate for transmitting
	 */
	protected function SetBaudRate($baudrate){
		if($this->baudRate!=$baudrate){
			$this->baudRate=$baudrate;
			$this->EncoderSend("BAUD:$baudrate\r\n");
			
		}
	}
	
	/**
	 * Sets the pager type (alpha or numeric).
	 * Can be 0-3. 0 is numeric, 3 is alphanumeric, 1 and 2 is tone only (I think).
	 * Determines the method used for encoding the message. No message is sent using 1 and 2, just the RIC header is sent
	 * @param int $type The pager type
	 */
	protected function SetPagerType($type){
		if($this->pagerType!=$type){
			$this->pagerType = $type;
			$this->EncoderSend("TYPE:$type\r\n");
		}
	}
	
	/**
	 * Sets the pager function bits.
	 * Pager function bits are a bit weird... They're sometimes used as extensions on the RIC, but on some pagers they change the tone cadence. Your milage may vary.
	 * @param int $function The function bits to use for transmit
	 */
	protected function SetPagerFunction($function){
		if($this->func!=$function){
			$this->func = $function;
			$this->EncoderSend("FUNC:$function\r\n");
		}
	}
	
	
	/**
	 * Sets the target pager RIC.
	 * @param int $ric Pager RIC
	 */
	protected function SetRIC($ric){
		if($this->RIC!=$ric){
			$this->RIC = $ric;
			$this->EncoderSend("RIC:$ric\r\n");
		}
	}
	
	
	/**
	 * Converts the frequency to be used to something the SI4432 module understands.
	 * Outputs a 3 byte hex that can be used by the encoder for setting up frequency and band
	 * @param double $frequency TX Frequency in MHz
	 * @return string frequency in hex for SI4432
	 */
	protected function FreqToSI4432Value($frequency){
		if(floor($frequency)<480){
			$hbsel = 0;
			$fbval = floor($frequency/10);
			$fbval-=24;
			if($fbval<0)$fbval=0;
		} else {
			$fbval = floor($frequency/20);
			$fbval-=24;
			if($fbval<0)$fbval=0;
			$hbsel = 1;
		}
		$fcval = (($frequency/(10*(1+$hbsel)))-$fbval-24)*64000;
		$fcval = dechex($fcval);
		$fbselect = dechex(64+(32*$hbsel)+$fbval);
		$hexvalue = strtoupper("$fbselect$fcval");
		$hexvalue = "0x$hexvalue";
		return $hexvalue;
	}
	
}
