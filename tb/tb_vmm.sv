`timescale 1ns/1ps
// Replay the paper vector and compare tick masks, signed totals and captured DMA frames.
module tb_vmm #(parameter RUN_TICKS=615);
  reg clk=0; always #5 clk=~clk;
  reg rst=1, tick=0, aresetn=0;
  reg [31:0] data=0;
  reg last=0, valid=0;
  wire ready;
  wire [3:0] packet;
  wire packet_valid, tc_error, sched_error, read_error, fifo_error;
  integer tick_index=0, c, i, fd, count=0;
  reg [31:0] packets[0:13];
  reg [15:0] expected[0:615];
  reg [15:0] seen=0;
  integer totals[0:15];
  wire [31:0] rxdata;
  wire [3:0] rxkeep;
  wire rxvalid,rxlast,overflow,duplicate;
  reg [7:0] rxcycle=0;
  wire rxready=(rxcycle[2:0]!=0);
  integer frames=0;
  ranc_tick_stream capture(.clk(clk),.rst(rst),.tick(tick),.packet(packet),.packet_valid(packet_valid),
    .m_axis_tdata(rxdata),.m_axis_tkeep(rxkeep),.m_axis_tlast(rxlast),.m_axis_tvalid(rxvalid),
    .m_axis_tready(rxready),.overflow(overflow),.duplicate(duplicate));
  RANCNetwork #(.GRID_DIMENSION_X(3),.GRID_DIMENSION_Y(3),
    .OUTPUT_CORE_X_COORDINATE(2),.OUTPUT_CORE_Y_COORDINATE(2),.NUM_OUTPUTS(16),
    .POTENTIAL_WIDTH(16),.MEMORY_FILEPATH("../../data/"),.MAXIMUM_NUMBER_OF_PACKETS(256)) dut (
    .clk(clk),.rst(rst),.tick(tick),.packet_out(packet),.packet_out_valid(packet_valid),
    .token_controller_error(tc_error),.scheduler_error(sched_error),
    .packet_read_error(read_error),.fifo_write_error(fifo_error),
    .s00_axis_aclk(clk),.s00_axis_aresetn(aresetn),.s00_axis_tdata(data),
    .s00_axis_tstrb(4'hf),.s00_axis_tlast(last),.s00_axis_tvalid(valid),.s00_axis_tready(ready));

  always @(posedge clk) begin
    rxcycle<=rxcycle+1'b1;
    if(!rst && rxvalid && rxready) begin
      if(rxdata[31:16] !== (16'(frames+1)) || rxdata[15:0] !== expected[frames] || rxkeep!==4'hf || !rxlast)
        $fatal(1,"DMA frame %0d mismatch, actual=%h",frames+1,rxdata);
      frames=frames+1;
    end
    #1;
    if (!rst) begin
      if(overflow || duplicate) $fatal(1,"capture error");
      if ({tc_error,sched_error,read_error,fifo_error} !== 4'b0000)
        $fatal(1,"hardware error at tick %0d: tc=%b scheduler=%b read=%b fifo=%b",tick_index,tc_error,sched_error,read_error,fifo_error);
      if (packet_valid) begin
        if ($isunknown(packet) || tick_index==0) $fatal(1,"invalid output");
        if (seen[packet]) $fatal(1,"duplicate channel %0d at tick %0d",packet,tick_index);
        seen[packet]=1;
        totals[packet]=totals[packet]+1;
        count=count+1;
        $fdisplay(fd,"%0d %0d",tick_index,packet);
      end
    end
  end
  task send(input [31:0] word,input bit final_word);
    integer waited;
    begin
      @(negedge clk); data=word; last=final_word; valid=1; waited=0;
      @(posedge clk);
      while (!ready) begin
        waited=waited+1;
        if(waited>1000) $fatal(1,"AXIS handshake timeout");
        @(posedge clk);
      end
      @(negedge clk); valid=0; last=0;
    end
  endtask
  initial begin
    for(i=0;i<16;i=i+1) totals[i]=0;
    $readmemh("../../data/input_packets.mem",packets);
    $readmemh("../../data/expected_spikes.mem",expected);
    fd=$fopen("rtl_packets.txt","w");
    repeat(10) @(negedge clk);
    rst=0; aresetn=1;
    repeat(20) @(negedge clk);
    // Continuous beats reproduce an actual MM2S DMA burst, holding data/valid
    // until TREADY accepts each word.
    @(negedge clk); valid=1;
    for(i=0;i<14;i=i+1) begin
      data=packets[i]; last=(i==13);
      @(posedge clk);
      while(!ready) @(posedge clk);
      @(negedge clk);
    end
    valid=0; last=0;
    repeat(1000) @(negedge clk);
    // CPU bus row t+1 is the set of packets produced during hardware tick t.
    for(tick_index=1;tick_index<=RUN_TICKS;tick_index=tick_index+1) begin
      seen=0;
      @(negedge clk); tick=1;
      @(negedge clk); tick=0;
      repeat(99999) @(negedge clk);
      if(seen !== expected[tick_index])
        $fatal(1,"tick %0d mismatch: RTL=%h CPU=%h",tick_index,seen,expected[tick_index]);
      if(tick_index%50==0) $display("VMM tick %0d/615 passed; packets=%0d",tick_index,count);
    end
    for(i=0;i<8;i=i+1) $display("VMM y[%0d]=%0d",i,totals[2*i]-totals[2*i+1]);
    if(RUN_TICKS==615 && count!=2632) $fatal(1,"wrong packet count %0d",count);
    if(frames!=RUN_TICKS) $fatal(1,"missing DMA frames: %0d",frames);
    $fclose(fd);
    $display("PASS: signed VMM RTL, %0d ticks, %0d packets, exact CPU spike vectors",RUN_TICKS,count);
    $finish;
  end
  initial begin #700000000; $fatal(1,"simulation watchdog"); end
endmodule
