
function set-computer-name( [string]$name )
{
    $ComputerInfo = Get-WmiObject -Class Win32_ComputerSystem
    $ComputerInfo.rename($name).ReturnValue
}

set-computer-name $args[0]

