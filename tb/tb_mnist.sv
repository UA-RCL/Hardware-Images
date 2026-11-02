`timescale 1ns/1ps
// Verify the exact migrated tutorial network, not the separate VMM mapping.
module tb_mnist #(
  parameter NUM_IMAGES=16, NUM_PACKETS=2464, NUM_ROWS=20,
  parameter FIXTURE_DIR="../../data/"
);
  reg clk=0; always #5 clk=~clk;
  reg rst=1, tick=0, valid=0, last=0;
  reg [31:0] data=0;
  wire ready, pv, tc_error, sc_error, rd_error, fi_error;
  wire [7:0] packet;
  reg [255:0] seen=0, expected[0:NUM_ROWS-1];
  reg [31:0] packets[0:NUM_PACKETS-1], counts[0:NUM_IMAGES-1];
  integer t=0, p, offset=0, spikes=0;
  RANCNetwork #(.GRID_DIMENSION_X(6), .GRID_DIMENSION_Y(1),
    .OUTPUT_CORE_X_COORDINATE(5), .OUTPUT_CORE_Y_COORDINATE(0),
    .NUM_OUTPUTS(256), .POTENTIAL_WIDTH(9),
    .MAXIMUM_NUMBER_OF_PACKETS(512), .MEMORY_FILEPATH("../../data/")) dut (
    .clk(clk), .rst(rst), .tick(tick), .packet_out(packet), .packet_out_valid(pv),
    .token_controller_error(tc_error), .scheduler_error(sc_error),
    .packet_read_error(rd_error), .fifo_write_error(fi_error),
    .s00_axis_aclk(clk), .s00_axis_aresetn(~rst), .s00_axis_tdata(data),
    .s00_axis_tstrb(4'hf), .s00_axis_tlast(last), .s00_axis_tvalid(valid),
    .s00_axis_tready(ready));
  always @(posedge clk) begin
    #1;
    if (!rst) begin
      if ({tc_error,sc_error,rd_error,fi_error} !== 4'b0)
        $fatal(1,"RANC error tick=%0d tc/sc/rd/fi=%b%b%b%b", t,tc_error,sc_error,rd_error,fi_error);
      if (pv) begin
        if ($isunknown(packet) || seen[packet]) $fatal(1,"Invalid/duplicate output");
        seen[packet]=1;
        spikes=spikes+1;
      end
    end
  end
  initial begin
    $readmemh({FIXTURE_DIR,"expected_spikes.mem"},expected);
    $readmemh({FIXTURE_DIR,"input_packets.mem"},packets);
    $readmemh({FIXTURE_DIR,"input_counts.mem"},counts);
    repeat(10) @(negedge clk); rst=0;
    repeat(20) @(negedge clk);
    if(expected[0] !== 256'b0) $fatal(1,"Expected initial silent reference row");
    for(t=1;t<NUM_ROWS;t=t+1) begin
      // Feed one image before advancing its scheduler tick. Remaining ticks drain.
      if(t<=NUM_IMAGES) begin
        for(p=0;p<counts[t-1];p=p+1) begin
          @(negedge clk); valid=1; data=packets[offset]; last=(p==counts[t-1]-1);
          @(posedge clk); while(!ready) @(posedge clk);
          offset=offset+1;
        end
        @(negedge clk); valid=0; last=0;
      end
      repeat(2000) @(negedge clk);
      seen=0; tick=1;
      @(negedge clk); tick=0;
      repeat(99999) @(negedge clk);
      if(seen !== expected[t]) $fatal(1,"Tick %0d mismatch RTL=%h CPU=%h", t,seen,expected[t]);
      $display("MNIST tick %0d matched, cumulative spikes=%0d",t,spikes);
    end
    $display("PASS: tutorial MNIST RTL matches %0d CPU rows; no accuracy claim",NUM_ROWS);
    $finish;
  end
  initial begin #25000000; $fatal(1,"Watchdog"); end
endmodule
