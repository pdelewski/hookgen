<?php

function pass($handle, $process) {
	if ($handle) {
		$index = 0;
		$child = "";
		$paren = "";
		$elements = array();
		while (($line = fgets($handle)) !== false) {			
			$output = array();
			parse_str($line, $output);
			if ($index % 2 == 0) {
				$child = $output['child'];		
			} else {
				$paren = $output['paren'];		
				$key = $child . $paren;
				if (!str_contains($key, "Composer") && !str_contains($key, "Autoloader") && !array_key_exists($key, $elements)) {
					$elements[$key] = 1;
					$process($child, $paren);
				}
			}
			++$index;
		}	
	}
}

$dump_vertices = function($child, $paren) {
	echo $child;
	echo $paren;
};

$dump_connections = function($child, $paren) {
	echo "\"" . $child . "\"" . "->" . "\"" . $paren . "\";";
};

$handle = fopen("./functions.log", "r");
//pass($handle, $dump_vertices);
fclose($handle);
$handle = fopen("./functions.log", "r");
echo "digraph functions{\n";
pass($handle, $dump_connections);
echo "}";
fclose($handle);
