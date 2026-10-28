`timescale 1ns/1ps
// One DMA word per tick, including silent ticks: {tick_id[15:0], spike_mask[15:0]}.
// FIFO decouples output capture from AXIS backpressure. Both clocks are identical.
module ranc_tick_stream #(
  parameter DEPTH=1024
)(
  input clk, input rst, input tick,
  input [3:0] packet, input packet_valid,
  output [31:0] m_axis_tdata, output [3:0] m_axis_tkeep,
  output m_axis_tlast, output m_axis_tvalid, input m_axis_tready,
  output reg overflow=0, output reg duplicate=0
);
  localparam AW=$clog2(DEPTH);
  reg [31:0] memory[0:DEPTH-1];
  reg [AW-1:0] wr=0, rd=0;
  reg [AW:0] count=0;
  reg [15:0] mask=0, tick_id=0;
  // A full FIFO can accept a tick when a downstream handshake frees a slot.
  wire pop=m_axis_tvalid && m_axis_tready;
  wire push=tick && ((count<DEPTH) || pop);
  assign m_axis_tdata=memory[rd];
  assign m_axis_tvalid=(count!=0);
  // Every word ends a packet, including silence; software rearms S2MM per word.
  assign m_axis_tlast=1'b1;
  assign m_axis_tkeep=4'hf;
  always @(posedge clk) begin
    if(rst) begin
      wr<=0; rd<=0; count<=0; mask<=0; tick_id<=0; overflow<=0; duplicate<=0;
    end else begin
      if(pop) rd<=rd+1'b1;
      if(tick) begin
        if(!push) overflow<=1;
        if(push) begin
          // Close the previous interval; tag 1 therefore contains initial silence.
          memory[wr]<={tick_id+16'd1,mask};
          wr<=wr+1'b1;
        end
        tick_id<=tick_id+1'b1;
        mask<=0;
      end
      if(packet_valid) begin
        // A bitmap cannot encode two spikes on one channel within an interval.
        if(mask[packet] && !tick) duplicate<=1;
        // A same-edge packet belongs to the new interval.
        if(tick) mask<=(16'b1<<packet);
        else mask[packet]<=1;
      end
      case({push,pop})
        2'b10:count<=count+1'b1;
        2'b01:count<=count-1'b1;
        default:count<=count;
      endcase
    end
  end
endmodule
