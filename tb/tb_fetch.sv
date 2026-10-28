`timescale 1ns/1ps
// Check known reset state, interrupted-fetch error detection and error recovery.
module tb_fetch;
  reg clk=0; always #5 clk=~clk;
  reg rst=1,tick=0,valid=0,full=0;
  reg [4:0] number=2;
  wire [4:0] addr;
  wire pv,error;
  PacketFetch #(.NUMBER_OF_INPUT_WORDS(32)) dut(.clk(clk),.tick(tick),.rst(rst),.num_packets(number),
    .data_valid(valid),.buffer_full(full),.addr(addr),.packet_valid(pv),.packet_read_error(error));
  initial begin
    repeat(3) @(negedge clk);
    if(error!==0) $fatal(1,"packet_read_error must be known zero after reset");
    rst=0;
    repeat(3) @(negedge clk);
    valid=1;
    @(negedge clk); valid=0; tick=1;
    @(negedge clk); tick=0;
    if(error!==1) $fatal(1,"early tick did not flag interrupted fetch");
    rst=1;
    repeat(2) @(negedge clk);
    if(error!==0) $fatal(1,"reset did not clear fetch error");
    $display("PASS: PacketFetch known reset state, early-tick error detection and reset recovery");
    $finish;
  end
endmodule
