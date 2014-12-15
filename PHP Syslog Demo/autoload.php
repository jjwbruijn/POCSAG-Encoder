<?php
function stdloader($class) {
    $path = 'inc/class.' . $class . '.inc.php';
    if(file_exists($path)){
        include $path;
    } else {
        $path = '../inc/class.' . $class . '.inc.php';
        if(file_exists($path)){
            include $path;
        }
    }
}
spl_autoload_register('stdloader');
