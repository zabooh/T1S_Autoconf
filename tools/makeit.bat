set MPLABX_TOOL_CHAIN=v6.15
set TOOL_PATH="c:\Program Files\Microchip\MPLABX\v6.15\gnuBins\GnuWin32\bin"

pushd .
cd ..\apps\tcpip_iperf_lan867x\firmware\tcpip_iperf_lan867x_freertos.X
%TOOL_PATH%\make CONF=FreeRTOS -j 8
popd

