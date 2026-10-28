`timescale 1ns/1ps
// Check the generated reset IP follows PS reset assertion/release and can reassert.
module tb_board_reset;
  reg clk=0;always #5 clk=~clk;
  reg ps_resetn=0;
  wire peripheral_resetn;
  ranc_system_reset_0 dut(.slowest_sync_clk(clk),.ext_reset_in(ps_resetn),.aux_reset_in(ps_resetn),
    .mb_debug_sys_rst(1'b0),.dcm_locked(1'b1),.mb_reset(),.bus_struct_reset(),.peripheral_reset(),
    .interconnect_aresetn(),.peripheral_aresetn(peripheral_resetn));
  initial begin
    repeat(30)@(negedge clk);
    if(peripheral_resetn!==0)$fatal(1,"PS reset assertion did not hold peripherals in reset");
    ps_resetn=1;
    repeat(100)@(negedge clk);
    if(peripheral_resetn!==1)$fatal(1,"peripherals stuck in reset after PS reset release");
    ps_resetn=0;
    repeat(30)@(negedge clk);
    if(peripheral_resetn!==0)$fatal(1,"PS reset did not reassert peripheral reset");
    $display("PASS: generated AUP reset IP asserts, releases, and reasserts peripheral reset with both inputs following PS resetn");
    $finish;
  end
endmodule
