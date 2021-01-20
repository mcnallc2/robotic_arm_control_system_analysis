connect -url tcp:127.0.0.1:3121
source /home/cmcnally/Repos/mai_project/Vivado/ultrasonic_sensors/ultrasonic_sensors.sdk/ultrasonic_unit_wrapper_hw_platform_0/ps7_init.tcl
targets -set -nocase -filter {name =~"APU*" && jtag_cable_name =~ "Xilinx TUL 1234-tulA"} -index 0
rst -system
after 3000
targets -set -filter {jtag_cable_name =~ "Xilinx TUL 1234-tulA" && level==0} -index 1
fpga -file /home/cmcnally/Repos/mai_project/Vivado/ultrasonic_sensors/ultrasonic_sensors.sdk/ultrasonic_unit_wrapper_hw_platform_0/ultrasonic_unit_wrapper.bit
targets -set -nocase -filter {name =~"APU*" && jtag_cable_name =~ "Xilinx TUL 1234-tulA"} -index 0
loadhw -hw /home/cmcnally/Repos/mai_project/Vivado/ultrasonic_sensors/ultrasonic_sensors.sdk/ultrasonic_unit_wrapper_hw_platform_0/system.hdf -mem-ranges [list {0x40000000 0xbfffffff}]
configparams force-mem-access 1
targets -set -nocase -filter {name =~"APU*" && jtag_cable_name =~ "Xilinx TUL 1234-tulA"} -index 0
ps7_init
ps7_post_config
targets -set -nocase -filter {name =~ "ARM*#0" && jtag_cable_name =~ "Xilinx TUL 1234-tulA"} -index 0
dow /home/cmcnally/Repos/mai_project/Vivado/ultrasonic_sensors/ultrasonic_sensors.sdk/ultrasonic_driver_bsp_xgpio_example_1/Debug/ultrasonic_driver_bsp_xgpio_example_1.elf
configparams force-mem-access 0
targets -set -nocase -filter {name =~ "ARM*#0" && jtag_cable_name =~ "Xilinx TUL 1234-tulA"} -index 0
con