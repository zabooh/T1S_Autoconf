set MPLABX_TOOL_CHAIN=v6.15

pushd .
cd ..\apps\tcpip_iperf_lan867x\firmware\tcpip_iperf_lan867x_freertos.X
make CONF=FreeRTOS -j 8
make CONF=FreeRTOS_node_1 -j 8
make CONF=FreeRTOS_node_2 -j 8
make CONF=FreeRTOS_node_3 -j 8
popd

