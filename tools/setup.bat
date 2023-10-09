set MPLABX_TOOL_CHAIN=v6.15

pushd .
cd ..\apps\tcpip_iperf_lan867x\firmware\tcpip_iperf_lan867x_freertos.X
"c:\Program Files\Microchip\MPLABX\%MPLABX_TOOL_CHAIN%\mplab_platform\bin\prjMakefilesGenerator.bat" .
popd



