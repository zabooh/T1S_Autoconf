rem SET REL_FILE=.\dist\FreeRTOS\debug\tcpip_iperf_lan867x_freertos.X.debug
SET REL_FILE=.\dist\FreeRTOS\production\tcpip_iperf_lan867x_freertos.X.production
SET TOOL="C:\Program Files\Microchip\xc32\v4.10\bin"

%TOOL%\xc32-readelf -s %REL_FILE%.elf > %REL_FILE%.sym
cmsort /S=7,100  %REL_FILE%.sym %REL_FILE%.symbols.txt
DEL %REL_FILE%.sym   
COPY %REL_FILE%.symbols.txt .
%TOOL%\xc32-objdump -S %REL_FILE%.elf > %REL_FILE%.disassembly.txt
COPY %REL_FILE%.disassembly.txt .
COPY %REL_FILE%.map .


