
# Set GT Pin location swapping message severity to "WARNING"
set_msg_config -id "Constraints 18-4427" -new_severity "WARNING"

reset_property LOC [get_cells -hierarchical -filter {NAME =~ *gen_channel_container[3].*gen_gtye4_channel_inst[*].GTYE4_CHANNEL_PRIM_INST}]
#reset_property LOC [get_cells -hierarchical -filter {NAME =~ *gen_channel_container[0].*gen_gthe4_channel_inst[1].GTHE4_CHANNEL_PRIM_INST}]
#reset_property LOC [get_cells -hierarchical -filter {NAME =~ *gen_channel_container[0].*gen_gthe4_channel_inst[2].GTHE4_CHANNEL_PRIM_INST}]
#reset_property LOC [get_cells -hierarchical -filter {NAME =~ *gen_channel_container[0].*gen_gthe4_channel_inst[3].GTHE4_CHANNEL_PRIM_INST}]

# Assign new LOC property according to d51 hdmi1 wiring
set_property LOC GTYE4_CHANNEL_X0Y14   [get_cells -hierarchical -filter {NAME =~ *gen_channel_container[3].*gen_gtye4_channel_inst[0].GTYE4_CHANNEL_PRIM_INST}]
set_property LOC GTYE4_CHANNEL_X0Y13   [get_cells -hierarchical -filter {NAME =~ *gen_channel_container[3].*gen_gtye4_channel_inst[1].GTYE4_CHANNEL_PRIM_INST}]
set_property LOC GTYE4_CHANNEL_X0Y12   [get_cells -hierarchical -filter {NAME =~ *gen_channel_container[3].*gen_gtye4_channel_inst[2].GTYE4_CHANNEL_PRIM_INST}]
set_property LOC GTYE4_CHANNEL_X0Y15   [get_cells -hierarchical -filter {NAME =~ *gen_channel_container[3].*gen_gtye4_channel_inst[3].GTYE4_CHANNEL_PRIM_INST}]
