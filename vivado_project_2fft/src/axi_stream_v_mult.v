`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/25/2026 06:12:22 PM
// Design Name: 
// Module Name: axi_stream_v_mult
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module axi_stream_v_mult (
    input  wire        aclk,
    input  wire        aresetn,

    // Slave Interface (H2C)
    input  wire [63:0] s_axis_tdata,
    input  wire [7:0]  s_axis_tkeep,
    input  wire        s_axis_tlast,
    input  wire        s_axis_tvalid,
    output wire        s_axis_tready,

    // Master Interface (C2H)
    output reg [63:0]  m_axis_tdata,
    output reg [7:0]   m_axis_tkeep,
    output reg         m_axis_tlast,
    output reg         m_axis_tvalid,
    input  wire        m_axis_tready
);

    // Only accept data if the master side is ready to take our next cycle output
    assign s_axis_tready = m_axis_tready || !m_axis_tvalid;

    always @(posedge aclk) begin
        if (!aresetn) begin
            m_axis_tvalid <= 1'b0;
            m_axis_tdata  <= 64'd0;
            m_axis_tlast  <= 1'b0;
            m_axis_tkeep  <= 8'h0;
        end else begin
            if (s_axis_tready) begin
                m_axis_tvalid <= s_axis_tvalid;
                m_axis_tlast  <= s_axis_tlast;
                m_axis_tkeep  <= s_axis_tkeep;
                // Pointwise Multiply: (Low 32-bits) * (High 32-bits)
                m_axis_tdata  <= s_axis_tdata[31:0] * s_axis_tdata[63:32];
            end
        end
    end

endmodule