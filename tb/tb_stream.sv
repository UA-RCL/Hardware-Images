`timescale 1ns/1ps
// Stress the capture FIFO under AXIS stalls; payload must stay stable until accepted.
module tb_stream;
  reg clk=0; always #5 clk=~clk;
  reg rst=1,tick=0,pv=0,ready=0;
  reg [3:0] packet=0;
  wire [31:0] data;
  wire [3:0] keep;
  wire last,valid,overflow,duplicate;
  integer cycle=0,i,j,got=0;
  reg blocked=0;
  reg [31:0] held=0;
  reg [15:0] mask;
  reg [31:0] expected[0:199];
  ranc_tick_stream #(.DEPTH(16)) dut(.clk(clk),.rst(rst),.tick(tick),.packet(packet),.packet_valid(pv),
    .m_axis_tdata(data),.m_axis_tkeep(keep),.m_axis_tlast(last),.m_axis_tvalid(valid),.m_axis_tready(ready),
    .overflow(overflow),.duplicate(duplicate));
  always @(negedge clk) begin
    cycle=cycle+1;
    ready=(cycle>100 && (cycle%13)<7);
  end
  always @(posedge clk) if(!rst) begin
    if(blocked && (!valid || data!==held)) $fatal(1,"AXIS payload changed while stalled");
    blocked=valid && !ready; held=data;
    if(valid && ready) begin
      if(got>=200 || data!==expected[got] || !last || keep!==4'hf)
        $fatal(1,"stream word %0d actual=%h expected=%h",got,data,expected[got]);
      got=got+1;
    end
    if(overflow || duplicate) $fatal(1,"unexpected capture error");
  end
  initial begin
    for(i=0;i<200;i=i+1) expected[i]={(16'(i+1)),(16'b1<<(i%16))|(16'b1<<((i+5)%16))};
    repeat(4) @(negedge clk); rst=0;
    for(j=0;j<200;j=j+1) begin
      @(negedge clk); packet=j%16; pv=1;
      @(negedge clk); packet=(j+5)%16;
      @(negedge clk); pv=0;
      @(negedge clk); tick=1;
      @(negedge clk); tick=0;
      repeat(3) @(negedge clk);
    end
    repeat(100) @(negedge clk);
    if(got!=200) $fatal(1,"missing output words %0d",got);
    $display("PASS: 200 tick-tagged DMA frames under AXIS backpressure, stable data and exact masks");
    $finish;
  end
  initial begin #100000; $fatal(1,"stream watchdog"); end
endmodule
