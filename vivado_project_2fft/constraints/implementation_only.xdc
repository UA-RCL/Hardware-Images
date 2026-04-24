# GT Pin location override for PCIe lanes (GTY Quad 128 / container[3])
set_msg_config -id "Constraints 18-4427" -new_severity "WARNING"
set_property LOC {} [get_cells -hierarchical \
    -filter {NAME =~ *gen_channel_container[3].*gen_gtye4_channel_inst[*].GTYE4_CHANNEL_PRIM_INST}]
set_property LOC GTYE4_CHANNEL_X0Y14 [get_cells -hierarchical \
    -filter {NAME =~ *gen_channel_container[3].*gen_gtye4_channel_inst[0].GTYE4_CHANNEL_PRIM_INST}]
set_property LOC GTYE4_CHANNEL_X0Y13 [get_cells -hierarchical \
    -filter {NAME =~ *gen_channel_container[3].*gen_gtye4_channel_inst[1].GTYE4_CHANNEL_PRIM_INST}]
set_property LOC GTYE4_CHANNEL_X0Y12 [get_cells -hierarchical \
    -filter {NAME =~ *gen_channel_container[3].*gen_gtye4_channel_inst[2].GTYE4_CHANNEL_PRIM_INST}]
set_property LOC GTYE4_CHANNEL_X0Y15 [get_cells -hierarchical \
    -filter {NAME =~ *gen_channel_container[3].*gen_gtye4_channel_inst[3].GTYE4_CHANNEL_PRIM_INST}]