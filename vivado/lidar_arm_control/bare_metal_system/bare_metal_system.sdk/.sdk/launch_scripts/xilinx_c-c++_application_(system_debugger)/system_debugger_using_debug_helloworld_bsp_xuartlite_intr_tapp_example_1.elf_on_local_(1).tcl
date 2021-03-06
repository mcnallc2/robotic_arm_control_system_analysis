connect -url tcp:127.0.0.1:3121
source /home/cmcnally/Repos/robotic_arm_control_system_analysis/vivado/lidar_arm_control/bare_metal_system/bare_metal_system.sdk/lidar_arm_control_wrapper_hw_platform_0/ps7_init.tcl
targets -set -nocase -filter {name =~"APU*" && jtag_cable_name =~ "Xilinx TUL 1234-tulA"} -index 0
loadhw -hw /home/cmcnally/Repos/robotic_arm_control_system_analysis/vivado/lidar_arm_control/bare_metal_system/bare_metal_system.sdk/lidar_arm_control_wrapper_hw_platform_0/system.hdf -mem-ranges [list {0x40000000 0xbfffffff}]
configparams force-mem-access 1
targets -set -nocase -filter {name =~"APU*" && jtag_cable_name =~ "Xilinx TUL 1234-tulA"} -index 0
stop
ps7_init
ps7_post_config
targets -set -nocase -filter {name =~ "ARM*#0" && jtag_cable_name =~ "Xilinx TUL 1234-tulA"} -index 0
rst -processor
targets -set -nocase -filter {name =~ "ARM*#0" && jtag_cable_name =~ "Xilinx TUL 1234-tulA"} -index 0
dow /home/cmcnally/Repos/robotic_arm_control_system_analysis/vivado/lidar_arm_control/bare_metal_system/bare_metal_system.sdk/lidar_xuartlite_intr_example/Debug/helloworld_bsp_xuartlite_intr_tapp_example_1.elf
configparams force-mem-access 0
targets -set -nocase -filter {name =~ "ARM*#0" && jtag_cable_name =~ "Xilinx TUL 1234-tulA"} -index 0
con
