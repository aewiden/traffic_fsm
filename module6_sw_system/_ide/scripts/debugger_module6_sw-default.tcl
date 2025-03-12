# Usage with Vitis IDE:
# In Vitis IDE create a Single Application Debug launch configuration,
# change the debug type to 'Attach to running target' and provide this 
# tcl script in 'Execute Script' option.
# Path of this script: /thayerfs/home/f005cpz/workspace/module6/module6_sw_system/_ide/scripts/debugger_module6_sw-default.tcl
# 
# 
# Usage with xsct:
# To debug using xsct, launch xsct and run below command
# source /thayerfs/home/f005cpz/workspace/module6/module6_sw_system/_ide/scripts/debugger_module6_sw-default.tcl
# 
connect -url tcp:127.0.0.1:3121
targets -set -nocase -filter {name =~"APU*"}
rst -system
after 3000
targets -set -filter {jtag_cable_name =~ "Digilent Zybo Z7 210351B7A99BA" && level==0 && jtag_device_ctx=="jsn-Zybo Z7-210351B7A99BA-13722093-0"}
fpga -file /thayerfs/home/f005cpz/workspace/module6/module6_sw/_ide/bitstream/module6_hw_wrapper.bit
targets -set -nocase -filter {name =~"APU*"}
loadhw -hw /thayerfs/home/f005cpz/workspace/module6/module6_hw_wrapper/export/module6_hw_wrapper/hw/module6_hw_wrapper.xsa -mem-ranges [list {0x40000000 0xbfffffff}] -regs
configparams force-mem-access 1
targets -set -nocase -filter {name =~"APU*"}
source /thayerfs/home/f005cpz/workspace/module6/module6_sw/_ide/psinit/ps7_init.tcl
ps7_init
ps7_post_config
targets -set -nocase -filter {name =~ "*A9*#0"}
dow /thayerfs/home/f005cpz/workspace/module6/module6_sw/Debug/module6_sw.elf
configparams force-mem-access 0
bpadd -addr &main
