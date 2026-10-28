`timescale 1ns/1ps
// Exhaustively check threshold/reset arithmetic and sampled neuron integration behavior.
module tb_units;
  reg signed [8:0] v,pt,nt,rp;
  reg mode;
  wire signed [8:0] out;
  wire spike;
  integer i,j,m,r,expected,checks=0;
  ThresholdResetUnit dut(.potential_in(v),.positive_threshold(pt),.negative_threshold(nt),
    .reset_potential(rp),.reset_mode(mode),.potential_out(out),.spike(spike));
  reg clk=0; always #5 clk=~clk;
  reg signed [8:0] leak=0, current=0;
  reg [35:0] weights={9'd1,9'h1ff,9'd16,9'h1f0};
  reg [1:0] inst=0;
  reg next_n=0,en=0,write_current=0;
  wire signed [8:0] nv;
  wire ns;
  integer u,k,w,nexpected;
  NeuronBlock neuron(.clk(clk),.leak(leak),.weights(weights),.positive_threshold(9'd1),
    .negative_threshold(9'd0),.reset_potential(9'd0),.current_potential(current),
    .neuron_instruction(inst),.reset_mode(1'b1),.next_neuron(next_n),.integrator_reg_en(en),
    .write_current_potential(write_current),.write_potential(nv),.spike_out(ns));
  // Match signed 9-bit wraparound when computing expected arithmetic results.
  function integer wrap9(input integer a);
    begin wrap9=((a+256)&511)-256; end
  endfunction
  initial begin
    for(m=0;m<2;m=m+1) for(j=1;j<=5;j=j+1) for(r=-2;r<=2;r=r+1) for(i=-256;i<256;i=i+1) begin
      v=i; pt=j; nt=-j; rp=r; mode=m;
      if(i>=j) expected=m?wrap9(i-j):r;
      else if(i<(-j)) expected=m?wrap9(i+j):-r;
      else expected=i;
      #1;
      if($signed(out)!==expected || spike!==(i>=j)) $fatal(1,"threshold/reset mismatch v=%0d mode=%0d",i,m);
      checks=checks+1;
    end
    for(i=0;i<1000;i=i+1) begin
      @(posedge clk); current=(i%401)-200; u=$signed(current); leak=(i%5)-2;
      next_n=1; write_current=1; en=1;
      @(negedge clk); #1;
      for(k=0;k<4;k=k+1) begin
        @(posedge clk); next_n=0; write_current=0; inst=k;
        case(k) 0:w=1; 1:w=-1; 2:w=16; 3:w=-16; endcase
        u=wrap9(u+w);
        @(negedge clk); #1;
      end
      u=wrap9(u+$signed(leak));
      // Linear negative reset subtracts negative_threshold (zero here),
      // so a negative potential is retained rather than clamped to zero.
      nexpected=(u>=1)?wrap9(u-1):u;
      if($signed(nv)!==nexpected || ns!==(u>=1)) $fatal(1,"neuron integration mismatch %0d actual=%0d spike=%b expected=%0d u=%0d",i,$signed(nv),ns,nexpected,u);
      @(posedge clk); en=0;
    end
    $display("PASS: %0d exhaustive threshold/reset checks and 1000 clocked signed LIF checks",checks);
    $finish;
  end
endmodule
