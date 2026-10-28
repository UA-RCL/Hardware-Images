`timescale 1ns/1ps
// PS-facing wrapper: GPIO controls logical ticks, AXIS carries spikes through DMA.
// Matrix/neuron configuration comes from memory files; x changes in each input burst.
module ranc_vmm_axis #(
  parameter MEMORY_FILEPATH="./",
  // Each core scans 256 axons for each of 256 neurons, plus control/routing cycles.
  // Cores run in parallel; a tick arriving before a scan ends raises controller error.
  // At ~100 MHz this gives 1 ms/tick; allow the dense neuron scan to finish.
  parameter TICK_CYCLES=100000
)(
  (* X_INTERFACE_INFO="xilinx.com:signal:clock:1.0 clk CLK", X_INTERFACE_PARAMETER="ASSOCIATED_BUSIF S_AXIS:M_AXIS, ASSOCIATED_RESET resetn, FREQ_HZ 99999985" *)
  input clk,
  (* X_INTERFACE_INFO="xilinx.com:signal:reset:1.0 resetn RST", X_INTERFACE_PARAMETER="POLARITY ACTIVE_LOW" *) input resetn,
  input [31:0] control,
  output [31:0] status,
  output tick_irq,
  (* X_INTERFACE_INFO="xilinx.com:interface:axis:1.0 S_AXIS TDATA" *) input [31:0] s_axis_tdata,
  (* X_INTERFACE_INFO="xilinx.com:interface:axis:1.0 S_AXIS TKEEP" *) input [3:0] s_axis_tkeep,
  (* X_INTERFACE_INFO="xilinx.com:interface:axis:1.0 S_AXIS TVALID" *) input s_axis_tvalid,
  (* X_INTERFACE_INFO="xilinx.com:interface:axis:1.0 S_AXIS TREADY" *) output s_axis_tready,
  (* X_INTERFACE_INFO="xilinx.com:interface:axis:1.0 S_AXIS TLAST" *) input s_axis_tlast,
  (* X_INTERFACE_INFO="xilinx.com:interface:axis:1.0 M_AXIS TDATA" *) output [31:0] m_axis_tdata,
  (* X_INTERFACE_INFO="xilinx.com:interface:axis:1.0 M_AXIS TKEEP" *) output [3:0] m_axis_tkeep,
  (* X_INTERFACE_INFO="xilinx.com:interface:axis:1.0 M_AXIS TVALID" *) output m_axis_tvalid,
  (* X_INTERFACE_INFO="xilinx.com:interface:axis:1.0 M_AXIS TREADY" *) input m_axis_tready,
  (* X_INTERFACE_INFO="xilinx.com:interface:axis:1.0 M_AXIS TLAST" *) output m_axis_tlast
);
  // control: bit 0=run, bit 1=soft reset, bits 31:16=tick limit (0=unlimited).
  // Soft reset clears control/FIFOs, not the initialized neuron potential RAM.
  wire rst=~resetn | control[1];
  reg [$clog2(TICK_CYCLES)-1:0] cycles=0;
  reg [15:0] ticks=0;
  reg tick=0;
  wire running=control[0] && (control[31:16]==0 || ticks<control[31:16]);
  // Generate ticks on the opposite edge from network/capture state updates.
  always @(negedge clk) begin
    if(rst) begin cycles<=0; ticks<=0; tick<=0; end
    else begin
      tick<=0;
      if(running) begin
        if(cycles==TICK_CYCLES-1) begin cycles<=0; tick<=1; ticks<=ticks+1'b1; end
        else cycles<=cycles+1'b1;
      end else cycles<=0;
    end
  end
  wire [3:0] packet;
  wire pv,tc_error,sc_error,rd_error,fi_error,overflow,duplicate;
  reg [3:0] sticky=0;
  always @(posedge clk) if(rst) sticky<=0; else sticky<=sticky|{fi_error,rd_error,sc_error,tc_error};
  // status[31:16]=ticks; [7:2]=duplicate/overflow/FIFO/fetch/scheduler/controller errors.
  // Errors remain visible until reset; [1:0] report running and current tick pulse.
  assign status={ticks,8'b0,duplicate,overflow,sticky,running,tick};
  assign tick_irq=tick;
  // Eight regular cores plus output (2,2); 16 channels form eight signed pairs.
  // 16-bit potentials prevent overflow of intermediate sums in this mapping.
  RANCNetwork #(.GRID_DIMENSION_X(3),.GRID_DIMENSION_Y(3),
    .OUTPUT_CORE_X_COORDINATE(2),.OUTPUT_CORE_Y_COORDINATE(2),.NUM_OUTPUTS(16),
    .POTENTIAL_WIDTH(16),.MEMORY_FILEPATH(MEMORY_FILEPATH),.MAXIMUM_NUMBER_OF_PACKETS(256)) network (
    .clk(clk),.rst(rst),.tick(tick),.packet_out(packet),.packet_out_valid(pv),
    .token_controller_error(tc_error),.scheduler_error(sc_error),.packet_read_error(rd_error),.fifo_write_error(fi_error),
    .s00_axis_aclk(clk),.s00_axis_aresetn(~rst),.s00_axis_tdata(s_axis_tdata),.s00_axis_tstrb(s_axis_tkeep),
    .s00_axis_tlast(s_axis_tlast),.s00_axis_tvalid(s_axis_tvalid),.s00_axis_tready(s_axis_tready));
  ranc_tick_stream capture(.clk(clk),.rst(rst),.tick(tick),.packet(packet),.packet_valid(pv),
    .m_axis_tdata(m_axis_tdata),.m_axis_tkeep(m_axis_tkeep),.m_axis_tlast(m_axis_tlast),
    .m_axis_tvalid(m_axis_tvalid),.m_axis_tready(m_axis_tready),.overflow(overflow),.duplicate(duplicate));
endmodule
