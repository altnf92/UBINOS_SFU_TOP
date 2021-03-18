Ubinos Example 02 (Basic Example for nRF5 Family Boards)
===============================================================================


Board : Custom board (nRF52840)

SMARTFARM Top board project.


The role of the bottom board is land control based on the RTC, which operates through feedback controls associated with sensors and actuators.
The role of the top board is to measure illumination and communicate with the user.
When an event MSG is received from the bottom board via UART, a BLE message is sent to the user.
When an option MSG is received from the user through BLE, the bottom board is informed of the variable settings such as feedback control condition.


SFU_main.c : main 

GT.., si115x.. : SI1133 driver code (TWI comm)

LAP_main.c : User lvl ble code
