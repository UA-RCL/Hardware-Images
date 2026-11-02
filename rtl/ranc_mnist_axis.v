`timescale 1ns/1ps
// CPU-controlled single steps allow SD/UART pauses without advancing images.
// One RX packet per completed step: {step_id, eight 32-bit channel-mask words}.
module ranc_mnist_axis #(
  parameter MEMORY_FILEPATH="../data/",
  parameter TICK_CYCLES=100000
)(
  input clk, input resetn, input [31:0] control, output [31:0] status,
  input [31:0] s_axis_tdata, input [3:0] s_axis_tkeep,
  input s_axis_tlast, input s_axis_tvalid, output s_axis_tready,
  output [31:0] m_axis_tdata, output [3:0] m_axis_tkeep,
  output m_axis_tlast, output m_axis_tvalid, input m_axis_tready,
  output tick_irq
);
  wire rst=~resetn | control[1];
  wire [7:0] packet;
  wire pv,tc_error,sc_error,rd_error,fi_error;
  reg tick=0, busy=0, pending=0, previous_go=0;
  reg [31:0] cycles=0;
  reg [15:0] step_id=0;
  reg [3:0] word_index=0;
  reg [255:0] mask=0, snapshot=0;
  reg [5:0] errors=0;
  wire request=control[0] & ~previous_go;
  assign status={step_id,8'b0,errors,busy,pending};
  assign tick_irq=tick;
  assign m_axis_tvalid=pending;
  assign m_axis_tkeep=4'hf;
  assign m_axis_tlast=(word_index==8);
  assign m_axis_tdata=(word_index==0)?{16'b0,step_id}:
                           snapshot[(word_index-1)*32 +: 32];
  always @(posedge clk) begin
    if(rst) begin
      tick<=0; busy<=0; pending<=0; previous_go<=0; cycles<=0;
      step_id<=0; word_index<=0; mask<=0; snapshot<=0; errors<=0;
    end else begin
      tick<=0; previous_go<=control[0];
      errors[3:0]<=errors[3:0]|{fi_error,rd_error,sc_error,tc_error};
      if(request) begin
        if(busy || pending) errors[4]<=1;
        else begin busy<=1; cycles<=0; mask<=0; end
      end
      if(busy) begin
        cycles<=cycles+1;
        // Allow complete burst fetch/routing before advancing the scheduler.
        if(cycles==2047) tick<=1;
        if(cycles==TICK_CYCLES+2048) begin
          busy<=0; snapshot<=mask; pending<=1; step_id<=step_id+1; word_index<=0;
        end
      end
      if(pv) begin
        if(!busy || mask[packet]) errors[5]<=1;
        mask[packet]<=1;
      end
      if(pending && m_axis_tready) begin
        if(word_index==8) begin pending<=0; word_index<=0; end
        else word_index<=word_index+1;
      end
    end
  end
  RANCNetwork #(.GRID_DIMENSION_X(6),.GRID_DIMENSION_Y(1),
    .OUTPUT_CORE_X_COORDINATE(5),.OUTPUT_CORE_Y_COORDINATE(0),.NUM_OUTPUTS(256),
    .POTENTIAL_WIDTH(9),.MAXIMUM_NUMBER_OF_PACKETS(512),
    .MEMORY_FILEPATH(MEMORY_FILEPATH)) network (
    .clk(clk),.rst(rst),.tick(tick),.packet_out(packet),.packet_out_valid(pv),
    .token_controller_error(tc_error),.scheduler_error(sc_error),
    .packet_read_error(rd_error),.fifo_write_error(fi_error),
    .s00_axis_aclk(clk),.s00_axis_aresetn(~rst),.s00_axis_tdata(s_axis_tdata),
    .s00_axis_tstrb(s_axis_tkeep),.s00_axis_tlast(s_axis_tlast),
    .s00_axis_tvalid(s_axis_tvalid),.s00_axis_tready(s_axis_tready));
endmodule
