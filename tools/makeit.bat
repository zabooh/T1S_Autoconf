set MPLABX_TOOL_CHAIN=v6.15

set HEX_FILE_01="..\apps\tcpip_iperf_lan867x\firmware\tcpip_iperf_lan867x_freertos.X\dist\FreeRTOS\production\tcpip_iperf_lan867x_freertos.X.production.hex"
set HEX_FILE_02="..\apps\tcpip_iperf_lan867x\firmware\tcpip_iperf_lan867x_freertos.X\dist\FreeRTOS_node_1\production\tcpip_iperf_lan867x_freertos.X.production.hex"
set HEX_FILE_03="..\apps\tcpip_iperf_lan867x\firmware\tcpip_iperf_lan867x_freertos.X\dist\FreeRTOS_node_2\production\tcpip_iperf_lan867x_freertos.X.production.hex"
set HEX_FILE_04="..\apps\tcpip_iperf_lan867x\firmware\tcpip_iperf_lan867x_freertos.X\dist\FreeRTOS_node_3\production\tcpip_iperf_lan867x_freertos.X.production.hex"
set MDB_PATH="c:\Program Files\Microchip\MPLABX\%MPLABX_TOOL_CHAIN%\mplab_platform\bin\mdb.bat"

pushd .
cd ..\apps\tcpip_iperf_lan867x\firmware\tcpip_iperf_lan867x_freertos.X
make CONF=FreeRTOS
make CONF=FreeRTOS_node_1
make CONF=FreeRTOS_node_2
make CONF=FreeRTOS_node_3
popd

