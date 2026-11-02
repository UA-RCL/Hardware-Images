`timescale 1ns/1ps
// Board-wrapper regression: TX burst, CPU step, 36-byte RX, stalled AXI output.
module tb_mnist_axis;
  reg clk=0; always #5 clk=~clk;
  reg resetn=0, valid=0, last=0;
  reg [31:0] control=0, data=0;
  wire ready,rxvalid,rxlast,irq;
  wire [31:0] rxdata,status;
  wire [3:0] keep;
  reg [7:0] cycle=0;
  wire rxready=cycle[2:0]!=0;
  reg [255:0] expected[0:19];
  reg [31:0] packets[0:2463],counts[0:15];
  integer t,p,w,offset=0,frames=0;
  reg [31:0] stalled_data;
  reg stalled=0;
  ranc_mnist_axis #(.MEMORY_FILEPATH("../../data/")) dut (
    .clk(clk),.resetn(resetn),.control(control),.status(status),
    .s_axis_tdata(data),.s_axis_tkeep(4'hf),.s_axis_tlast(last),
    .s_axis_tvalid(valid),.s_axis_tready(ready),
    .m_axis_tdata(rxdata),.m_axis_tkeep(keep),.m_axis_tlast(rxlast),
    .m_axis_tvalid(rxvalid),.m_axis_tready(rxready),.tick_irq(irq));
  always @(posedge clk) begin
    cycle<=cycle+1;
    if(resetn) begin
      if(status[7:2] !== 6'b0) $fatal(1,"Wrapper error %h",status);
      if(stalled && (!rxvalid || rxdata!==stalled_data)) $fatal(1,"Unstable stalled data");
      stalled=rxvalid && !rxready;
      stalled_data=rxdata;
    end
  end
  initial begin
    $readmemh("../../data/expected_spikes.mem",expected);
    $readmemh("../../data/input_packets.mem",packets);
    $readmemh("../../data/input_counts.mem",counts);
    repeat(10) @(negedge clk); resetn=1;
    repeat(20) @(negedge clk);
    for(t=1;t<20;t=t+1) begin
      if(t<=16) begin
        for(p=0;p<counts[t-1];p=p+1) begin
          @(negedge clk); valid=1; data=packets[offset]; last=(p==counts[t-1]-1);
          @(posedge clk); while(!ready) @(posedge clk);
          offset=offset+1;
        end
        @(negedge clk); valid=0; last=0;
      end
      @(negedge clk); control=1;
      w=0;
      while(w<9) begin
        @(posedge clk);
        if(rxvalid && rxready) begin
          if(keep!==4'hf || rxlast!==(w==8)) $fatal(1,"Bad AXI framing");
          if(w==0) begin
            if(rxdata!==t) $fatal(1,"Bad step tag");
          end else if(rxdata!==expected[t][(w-1)*32 +:32])
            $fatal(1,"Step %0d mask word %0d mismatch got=%h expected=%h",t,w,rxdata,expected[t][(w-1)*32 +:32]);
          w=w+1;
        end
      end
      @(negedge clk); control=0;
      repeat(4) @(negedge clk);
      if(status[31:16]!==16'(t) || status[1:0]!==2'b0) $fatal(1,"Incomplete step");
      frames=frames+1;
      $display("MNIST AXI step %0d matched",t);
    end
    $display("PASS: MNIST board wrapper 19 complete 36-byte frames under backpressure");
    $finish;
  end
  initial begin #25000000; $fatal(1,"Wrapper watchdog"); end
endmodule
