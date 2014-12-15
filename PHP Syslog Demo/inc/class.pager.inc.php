<?php

class pager{
	
	protected $sql;
	
	/**
	 * The Pagers RIC or Radio Identification Code
	 * @var int
	 */
	public $RIC;
	
	/**
	 * Baudrate to be used for transmit
	 * @var int 
	 */
	public $baudRate;
	
	/**
	 * Transmit frequency in MHz
	 * @var double
	 */
	public $frequency;
	
	/**
	 * Pager function bits to address. What this does varies from pager to pager
	 * @var int
	 */
	public $func;
	
	/**
	 * Pagertype, 3 for alpha, 0 for numeric, 1 and 2 for tone only
	 * @var int
	 */
	public $pagerType;
	
	/**
	 * Pager model, some extra info I guess...
	 * @var string
	 */
	public $model;
	
	/**
	 * Saves the pagers info and settings based on RIC
	 */
	public function Save(){
		
	}
	
	/**
	 * Attempts to find information based on RIC
	 * @param int $ric The RIC to find
	 * @return bool true if successful
	 */	
	public function Lookup($ric){
		$this->RIC = 10238;
		$this->baudRate = 1200;
		$this->frequency = 460.91875;
		$this->func = 2;
		$this->pagerType = 3;
		return true;
	}
	
	/**
	 * Finds and returns a pager object based on the supplied RIC
	 * @param int $ric the RIC to find
	 * @return object|false The pager object, or false if it can't be found.
	 */
	public static function ByRIC($ric){
		$pager = new pager;
		if($pager->Lookup($ric)){
			return $pager;
		} else {
			return false;
		}
	}
	
	/**
	 * Return a hardcoded emergency pager. Useful if the SQL connection fails...
	 */
	public static function EmergencyPager(){
		$pager = new pager;
		$pager->RIC = 10238;
		$pager->baudRate = 1200;
		$pager->frequency = 460.91875;
		$pager->func = 2;
		$pager->pagerType = 3;
		$pager->model = "Nec A20";
		return $pager;
	}
}
