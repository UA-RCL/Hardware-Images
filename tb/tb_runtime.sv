`timescale 1ns/1ps
// Two different runtime vectors on the same fixed matrix, with no reset or
// memory reload between vectors. Both signs are exercised in both runs.
// Drain each vector fully before the next injection; no bitstream or matrix reload.
module tb_runtime;
  reg clk=0;always #5 clk=~clk;
  reg rst=1,tick=0,aresetn=0,valid=0,last=0;
  reg [31:0] data=0;
  wire ready,pv,te,se,re,fe;
  wire [3:0] packet;
  reg [31:0] packets[0:3],expected_y[0:7];
  reg [15:0] expected[0:188],seen=0;
  integer phase=0,t=0,i,count=0;
  integer totals[0:15];
  RANCNetwork #(.GRID_DIMENSION_X(3),.GRID_DIMENSION_Y(3),.OUTPUT_CORE_X_COORDINATE(2),
    .OUTPUT_CORE_Y_COORDINATE(2),.NUM_OUTPUTS(16),.POTENTIAL_WIDTH(16),
    .MEMORY_FILEPATH("../../data/"),.MAXIMUM_NUMBER_OF_PACKETS(256)) dut(
    .clk(clk),.rst(rst),.tick(tick),.packet_out(packet),.packet_out_valid(pv),
    .token_controller_error(te),.scheduler_error(se),.packet_read_error(re),.fifo_write_error(fe),
    .s00_axis_aclk(clk),.s00_axis_aresetn(aresetn),.s00_axis_tdata(data),.s00_axis_tstrb(4'hf),
    .s00_axis_tlast(last),.s00_axis_tvalid(valid),.s00_axis_tready(ready));
  always @(posedge clk) begin
    #1;
    if(!rst) begin
      if({te,se,re,fe}!==4'b0000) $fatal(1,"runtime hardware error phase=%0d",phase);
      if(pv) begin
        if(seen[packet]) $fatal(1,"duplicate runtime output");
        seen[packet]=1;
        totals[packet]=totals[packet]+1;
      end
    end
  end
  initial begin
    repeat(10) @(negedge clk);rst=0;aresetn=1;
    repeat(20) @(negedge clk);
    for(phase=1;phase<=2;phase=phase+1) begin
      if(phase==1) begin
        $readmemh("../../data/vector_a/input_packets.mem",packets);
        $readmemh("../../data/vector_a/expected_spikes.mem",expected);
        $readmemh("../../data/vector_a/expected_result.mem",expected_y);
      end else begin
        $readmemh("../../data/vector_b/input_packets.mem",packets);
        $readmemh("../../data/vector_b/expected_spikes.mem",expected);
        $readmemh("../../data/vector_b/expected_result.mem",expected_y);
      end
      for(i=0;i<16;i=i+1)totals[i]=0;
      @(negedge clk);valid=1;
      for(i=0;i<4;i=i+1) begin
        data=packets[i];last=(i==3);
        @(posedge clk);while(!ready)@(posedge clk);
        @(negedge clk);
      end
      valid=0;last=0;
      repeat(1000)@(negedge clk);
      for(t=1;t<=188;t=t+1) begin
        seen=0;
        @(negedge clk);tick=1;
        @(negedge clk);tick=0;
        repeat(99999)@(negedge clk);
        if(seen!==expected[t])$fatal(1,"runtime phase %0d tick %0d actual=%h expected=%h",phase,t,seen,expected[t]);
        if(t%50==0)$display("Runtime vector %0d: %0d/188 ticks passed",phase,t);
      end
      for(i=0;i<8;i=i+1) begin
        if(totals[2*i]-totals[2*i+1]!==$signed(expected_y[i]))$fatal(1,"runtime vector result mismatch");
        $display("Runtime vector %0d y[%0d]=%0d",phase,i,totals[2*i]-totals[2*i+1]);
      end
    end
    $display("PASS: two different runtime vectors, fixed matrix, no reset or memory reload between runs");
    $finish;
  end
  initial begin #450000000;$fatal(1,"runtime watchdog");end
endmodule
