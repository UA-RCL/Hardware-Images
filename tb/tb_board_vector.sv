`timescale 1ns/1ps
// Exercise the actual board wrapper/control/tick/capture path from fresh PL state.
// Compare each {tick,mask} frame and decoded signed totals against generated references.
module tb_board_vector #(parameter RUN_FRAMES=262);
  reg clk=0;always #5 clk=~clk;
  reg resetn=0,valid=0,last=0,rxready=0;
  reg [31:0] control=0,data=0;
  wire ready,rxvalid,rxlast,irq;
  wire [31:0] status,rxdata;
  wire [3:0] rxkeep;
  reg [31:0] packets[0:1];
  reg [15:0] expected[0:261];
  reg [31:0] expected_y[0:7];
  integer frames=0,i,j;
  integer totals[0:15];
  ranc_vmm_axis #(.MEMORY_FILEPATH("../../data/")) dut(
    .clk(clk),.resetn(resetn),.control(control),.status(status),.tick_irq(irq),
    .s_axis_tdata(data),.s_axis_tkeep(4'hf),.s_axis_tvalid(valid),.s_axis_tready(ready),.s_axis_tlast(last),
    .m_axis_tdata(rxdata),.m_axis_tkeep(rxkeep),.m_axis_tvalid(rxvalid),.m_axis_tready(rxready),.m_axis_tlast(rxlast));
  always @(posedge clk) if(resetn && !control[1]) begin
    if(status[7:2]!==0)$fatal(1,"board wrapper error status=%h",status);
    if(rxvalid && rxready)begin
      if(rxdata !== ((32'(frames+1)<<16)|expected[frames]) || !rxlast || rxkeep!==4'hf)
        $fatal(1,"board wrapper frame %0d actual=%h expected=%h",frames+1,rxdata,((32'(frames+1)<<16)|expected[frames]));
      for(j=0;j<16;j=j+1)totals[j]=totals[j]+rxdata[j];
      if(frames<12 || (frames+1)%50==0)$display("Board frame %0d passed: %h",frames+1,rxdata);
      frames=frames+1;
      if(frames==RUN_FRAMES)begin
        if(RUN_FRAMES==262)for(j=0;j<8;j=j+1)begin
          if(totals[2*j]-totals[2*j+1]!==$signed(expected_y[j]))$fatal(1,"board signed result mismatch column %0d",j);
          $display("Board y[%0d]=%0d",j,totals[2*j]-totals[2*j+1]);
        end
        $display("PASS: fresh board wrapper x=[0,0,0,0,1], %0d frames",frames);$finish;
      end
    end
  end
  initial begin
    $readmemh("../../data/unit_vector_x4/input_packets.mem",packets);
    $readmemh("../../data/unit_vector_x4/expected_spikes.mem",expected);
    $readmemh("../../data/unit_vector_x4/expected_result.mem",expected_y);
    for(i=0;i<16;i=i+1)totals[i]=0;
    repeat(10)@(negedge clk);resetn=1;
    // Reproduce initialization + run_case() controller resets, with ticks stopped.
    control=2;repeat(1000)@(negedge clk);control=0;repeat(1000)@(negedge clk);
    control=2;repeat(1000)@(negedge clk);control=0;repeat(1000)@(negedge clk);
    rxready=1;valid=1;
    for(i=0;i<2;i=i+1)begin
      data=packets[i];last=(i==1);
      @(posedge clk);while(!ready)@(posedge clk);
      @(negedge clk);
    end
    valid=0;last=0;
    control=(262<<16)|1;
  end
  initial begin #300000000;$fatal(1,"board vector watchdog");end
endmodule
