<?php
use PHPUnit\Framework\TestCase;

// import @PHP_PORT_NAME@

class MetaCallTest extends TestCase
{
	public function python()
	{
		$this->assertSame(0, metacall_load_from_file('py', ['example.py']));

		$this->assertSame(null, metacall('hello'));

		$this->assertSame(35, metacall('multiply', 5, 7));
	}
}
?>
