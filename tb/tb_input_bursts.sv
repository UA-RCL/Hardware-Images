`timescale 1ns/1ps
// Regression for short/long AXIS bursts: fetch only after accepted TLAST,
// preserve every word across TVALID gaps, and reject unaccepted completion.
module tb_input_bursts;
  reg clk=0;always #5 clk=~clk;
  reg rst=1,tick=0,valid=0,last=0;
  reg [31:0] data=0;
  wire ready,done,pv,fe,re;
  wire [7:0] addr,count;
  wire [31:0] fetched;
  reg [31:0] words[0:254];
  integer lengths[0:6];
  integer phase=0,n=0,i=0,seen=0,accepted=0,completions=0,waited=0;
  reg completed=0;
  RANCNetwork_S00_AXIS #(.NUMBER_OF_INPUT_WORDS(256)) axis(
    .tick(tick),.writes_done_out(done),.dout(fetched),.addr(addr),.num_packets(count),.fifo_write_error(fe),
    .S_AXIS_ACLK(clk),.S_AXIS_ARESETN(!rst),.S_AXIS_TREADY(ready),.S_AXIS_TDATA(data),
    .S_AXIS_TSTRB(4'hf),.S_AXIS_TLAST(last),.S_AXIS_TVALID(valid));
  PacketFetch #(.NUMBER_OF_INPUT_WORDS(256)) fetch(
    .clk(clk),.tick(tick),.rst(rst),.num_packets(count),.data_valid(done),.buffer_full(1'b0),
    .addr(addr),.packet_valid(pv),.packet_read_error(re));
  always @(posedge clk)if(!rst)begin
    if(valid && ready)begin accepted=accepted+1;if(last)completed=1;end
    if(done)completions=completions+1;
    if(pv)begin
      if(!completed)$fatal(1,"fetch started before accepted TLAST");
      if(seen>=n || fetched!==words[seen])$fatal(1,"burst %0d word %0d actual=%h",n,seen,fetched);
      seen=seen+1;
    end
    if(fe || re)$fatal(1,"input transport error");
  end
  initial begin
    lengths[0]=1;lengths[1]=2;lengths[2]=3;lengths[3]=4;
    lengths[4]=14;lengths[5]=150;lengths[6]=255;
    repeat(5)@(negedge clk);rst=0;repeat(5)@(negedge clk);
    for(phase=0;phase<7;phase=phase+1)begin
      n=lengths[phase];seen=0;accepted=0;completions=0;completed=0;
      // TLAST is meaningless without an accepted beat.
      last=1;repeat(3)@(negedge clk);last=0;
      if(done || seen || completions)$fatal(1,"unaccepted TLAST completed a burst");
      for(i=0;i<n;i=i+1)words[i]=32'h10000000|(phase<<16)|i;
      for(i=0;i<n;i=i+1)begin
        valid=1;data=words[i];last=(i==n-1);
        @(posedge clk);while(!ready)@(posedge clk);
        @(negedge clk);
        // Include TVALID gaps; the loader must wait for the entire burst.
        if(i%3==0 && i+1<n)begin valid=0;last=0;repeat(2)@(negedge clk);end
      end
      valid=0;last=0;waited=0;
      while(seen<n && waited<1000)begin @(negedge clk);waited=waited+1;end
      repeat(10)@(negedge clk);
      if(seen!=n || accepted!=n || completions!=1)$fatal(1,"burst n=%0d accepted=%0d forwarded=%0d completions=%0d",n,accepted,seen,completions);
      $display("Input burst %0d: every packet forwarded once",n);
      tick=1;@(negedge clk);tick=0;repeat(10)@(negedge clk);
    end
    $display("PASS: complete AXIS bursts 1,2,3,4,14,150,255; VALID gaps; unaccepted TLAST; no packet loss/duplication");
    $finish;
  end
  initial begin #1000000;$fatal(1,"input burst watchdog");end
endmodule
