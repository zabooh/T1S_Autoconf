set MPLABX_TOOL_CHAIN=v6.15

set HEX_FILE_01="..\apps\tcpip_iperf_lan867x\firmware\tcpip_iperf_lan867x_freertos.X\dist\FreeRTOS\production\tcpip_iperf_lan867x_freertos.X.production.hex"
set HEX_FILE_02="..\apps\tcpip_iperf_lan867x\firmware\tcpip_iperf_lan867x_freertos.X\dist\FreeRTOS_node_1\production\tcpip_iperf_lan867x_freertos.X.production.hex"
set MDB_PATH="c:\Program Files\Microchip\MPLABX\%MPLABX_TOOL_CHAIN%\mplab_platform\bin\mdb.bat"

set HW_TOOL="EDBG"
set TG_MCU="ATSAME54P20A"
set HW_SERIAL_01="ATML3264031800001049"
set HW_SERIAL_02="ATML3264031800001044"

pushd .
cd ..\apps\tcpip_iperf_lan867x\firmware\tcpip_iperf_lan867x_freertos.X
make CONF=FreeRTOS
make CONF=FreeRTOS_node_1
popd
pause

START /B python -u mdb_flash.py --hex=%HEX_FILE_01% --mdb-path=%MDB_PATH% --mcu=%TG_MCU% --hwtool=%HW_TOOL% --hwtool-serial=%HW_SERIAL_01%
START /B python -u mdb_flash.py --hex=%HEX_FILE_02% --mdb-path=%MDB_PATH% --mcu=%TG_MCU% --hwtool=%HW_TOOL% --hwtool-serial=%HW_SERIAL_02%
