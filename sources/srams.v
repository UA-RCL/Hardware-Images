
module data_arrays_0_ext(
  input RW0_clk,
  input [7:0] RW0_addr,
  input RW0_en,
  input RW0_wmode,
  input [7:0] RW0_wmask,
  input [63:0] RW0_wdata,
  output [63:0] RW0_rdata
);

  reg [7:0] ram0 [255:0];
  reg [7:0] ram1 [255:0];
  reg [7:0] ram2 [255:0];
  reg [7:0] ram3 [255:0];
  reg [7:0] ram4 [255:0];
  reg [7:0] ram5 [255:0];
  reg [7:0] ram6 [255:0];
  reg [7:0] ram7 [255:0];
  reg [7:0] reg_RW0_addr;

  assign RW0_rdata[7:0] = ram0[reg_RW0_addr];
  assign RW0_rdata[15:8] = ram1[reg_RW0_addr];
  assign RW0_rdata[23:16] = ram2[reg_RW0_addr];
  assign RW0_rdata[31:24] = ram3[reg_RW0_addr];
  assign RW0_rdata[39:32] = ram4[reg_RW0_addr];
  assign RW0_rdata[47:40] = ram5[reg_RW0_addr];
  assign RW0_rdata[55:48] = ram6[reg_RW0_addr];
  assign RW0_rdata[63:56] = ram7[reg_RW0_addr];

  always @(posedge RW0_clk) begin
    if (RW0_en) begin
      if (RW0_wmode) begin
        if (RW0_wmask[0]) ram0[RW0_addr] <= RW0_wdata[7:0];
        if (RW0_wmask[1]) ram1[RW0_addr] <= RW0_wdata[15:8];
        if (RW0_wmask[2]) ram2[RW0_addr] <= RW0_wdata[23:16];
        if (RW0_wmask[3]) ram3[RW0_addr] <= RW0_wdata[31:24];
        if (RW0_wmask[4]) ram4[RW0_addr] <= RW0_wdata[39:32];
        if (RW0_wmask[5]) ram5[RW0_addr] <= RW0_wdata[47:40];
        if (RW0_wmask[6]) ram6[RW0_addr] <= RW0_wdata[55:48];
        if (RW0_wmask[7]) ram7[RW0_addr] <= RW0_wdata[63:56];
      end else begin
        reg_RW0_addr <= RW0_addr;
      end
    end
  end

endmodule

module tag_array_ext(
  input RW0_clk,
  input [4:0] RW0_addr,
  input RW0_en,
  input RW0_wmode,
  input [24:0] RW0_wdata,
  output [24:0] RW0_rdata
);

  reg [24:0] ram0 [31:0];
  reg [4:0] reg_RW0_addr;

  assign RW0_rdata[24:0] = ram0[reg_RW0_addr];

  always @(posedge RW0_clk) begin
    if (RW0_en) begin
      if (RW0_wmode) begin
        ram0[RW0_addr] <= RW0_wdata[24:0];
      end else begin
        reg_RW0_addr <= RW0_addr;
      end
    end
  end

endmodule

module tag_array_0_ext(
  input RW0_clk,
  input [4:0] RW0_addr,
  input RW0_en,
  input RW0_wmode,
  input [0:0] RW0_wmask,
  input [23:0] RW0_wdata,
  output [23:0] RW0_rdata
);

  reg [23:0] ram0 [31:0];
  reg [4:0] reg_RW0_addr;

  assign RW0_rdata[23:0] = ram0[reg_RW0_addr];

  always @(posedge RW0_clk) begin
    if (RW0_en) begin
      if (RW0_wmode) begin
        if (RW0_wmask[0]) ram0[RW0_addr] <= RW0_wdata[23:0];
      end else begin
        reg_RW0_addr <= RW0_addr;
      end
    end
  end

endmodule

module data_arrays_0_0_ext(
  input RW0_clk,
  input [7:0] RW0_addr,
  input RW0_en,
  input RW0_wmode,
  input [0:0] RW0_wmask,
  input [31:0] RW0_wdata,
  output [31:0] RW0_rdata
);

  reg [31:0] ram0 [255:0];
  reg [7:0] reg_RW0_addr;

  assign RW0_rdata[31:0] = ram0[reg_RW0_addr];

  always @(posedge RW0_clk) begin
    if (RW0_en) begin
      if (RW0_wmode) begin
        if (RW0_wmask[0]) ram0[RW0_addr] <= RW0_wdata[31:0];
      end else begin
        reg_RW0_addr <= RW0_addr;
      end
    end
  end

endmodule

module data_arrays_0_1_ext(
  input RW0_clk,
  input [8:0] RW0_addr,
  input RW0_en,
  input RW0_wmode,
  input [31:0] RW0_wmask,
  input [255:0] RW0_wdata,
  output [255:0] RW0_rdata
);

  reg [7:0] ram0 [511:0];
  reg [7:0] ram1 [511:0];
  reg [7:0] ram2 [511:0];
  reg [7:0] ram3 [511:0];
  reg [7:0] ram4 [511:0];
  reg [7:0] ram5 [511:0];
  reg [7:0] ram6 [511:0];
  reg [7:0] ram7 [511:0];
  reg [7:0] ram8 [511:0];
  reg [7:0] ram9 [511:0];
  reg [7:0] ram10 [511:0];
  reg [7:0] ram11 [511:0];
  reg [7:0] ram12 [511:0];
  reg [7:0] ram13 [511:0];
  reg [7:0] ram14 [511:0];
  reg [7:0] ram15 [511:0];
  reg [7:0] ram16 [511:0];
  reg [7:0] ram17 [511:0];
  reg [7:0] ram18 [511:0];
  reg [7:0] ram19 [511:0];
  reg [7:0] ram20 [511:0];
  reg [7:0] ram21 [511:0];
  reg [7:0] ram22 [511:0];
  reg [7:0] ram23 [511:0];
  reg [7:0] ram24 [511:0];
  reg [7:0] ram25 [511:0];
  reg [7:0] ram26 [511:0];
  reg [7:0] ram27 [511:0];
  reg [7:0] ram28 [511:0];
  reg [7:0] ram29 [511:0];
  reg [7:0] ram30 [511:0];
  reg [7:0] ram31 [511:0];
  reg [8:0] reg_RW0_addr;

  assign RW0_rdata[7:0] = ram0[reg_RW0_addr];
  assign RW0_rdata[15:8] = ram1[reg_RW0_addr];
  assign RW0_rdata[23:16] = ram2[reg_RW0_addr];
  assign RW0_rdata[31:24] = ram3[reg_RW0_addr];
  assign RW0_rdata[39:32] = ram4[reg_RW0_addr];
  assign RW0_rdata[47:40] = ram5[reg_RW0_addr];
  assign RW0_rdata[55:48] = ram6[reg_RW0_addr];
  assign RW0_rdata[63:56] = ram7[reg_RW0_addr];
  assign RW0_rdata[71:64] = ram8[reg_RW0_addr];
  assign RW0_rdata[79:72] = ram9[reg_RW0_addr];
  assign RW0_rdata[87:80] = ram10[reg_RW0_addr];
  assign RW0_rdata[95:88] = ram11[reg_RW0_addr];
  assign RW0_rdata[103:96] = ram12[reg_RW0_addr];
  assign RW0_rdata[111:104] = ram13[reg_RW0_addr];
  assign RW0_rdata[119:112] = ram14[reg_RW0_addr];
  assign RW0_rdata[127:120] = ram15[reg_RW0_addr];
  assign RW0_rdata[135:128] = ram16[reg_RW0_addr];
  assign RW0_rdata[143:136] = ram17[reg_RW0_addr];
  assign RW0_rdata[151:144] = ram18[reg_RW0_addr];
  assign RW0_rdata[159:152] = ram19[reg_RW0_addr];
  assign RW0_rdata[167:160] = ram20[reg_RW0_addr];
  assign RW0_rdata[175:168] = ram21[reg_RW0_addr];
  assign RW0_rdata[183:176] = ram22[reg_RW0_addr];
  assign RW0_rdata[191:184] = ram23[reg_RW0_addr];
  assign RW0_rdata[199:192] = ram24[reg_RW0_addr];
  assign RW0_rdata[207:200] = ram25[reg_RW0_addr];
  assign RW0_rdata[215:208] = ram26[reg_RW0_addr];
  assign RW0_rdata[223:216] = ram27[reg_RW0_addr];
  assign RW0_rdata[231:224] = ram28[reg_RW0_addr];
  assign RW0_rdata[239:232] = ram29[reg_RW0_addr];
  assign RW0_rdata[247:240] = ram30[reg_RW0_addr];
  assign RW0_rdata[255:248] = ram31[reg_RW0_addr];

  always @(posedge RW0_clk) begin
    if (RW0_en) begin
      if (RW0_wmode) begin
        if (RW0_wmask[0]) ram0[RW0_addr] <= RW0_wdata[7:0];
        if (RW0_wmask[1]) ram1[RW0_addr] <= RW0_wdata[15:8];
        if (RW0_wmask[2]) ram2[RW0_addr] <= RW0_wdata[23:16];
        if (RW0_wmask[3]) ram3[RW0_addr] <= RW0_wdata[31:24];
        if (RW0_wmask[4]) ram4[RW0_addr] <= RW0_wdata[39:32];
        if (RW0_wmask[5]) ram5[RW0_addr] <= RW0_wdata[47:40];
        if (RW0_wmask[6]) ram6[RW0_addr] <= RW0_wdata[55:48];
        if (RW0_wmask[7]) ram7[RW0_addr] <= RW0_wdata[63:56];
        if (RW0_wmask[8]) ram8[RW0_addr] <= RW0_wdata[71:64];
        if (RW0_wmask[9]) ram9[RW0_addr] <= RW0_wdata[79:72];
        if (RW0_wmask[10]) ram10[RW0_addr] <= RW0_wdata[87:80];
        if (RW0_wmask[11]) ram11[RW0_addr] <= RW0_wdata[95:88];
        if (RW0_wmask[12]) ram12[RW0_addr] <= RW0_wdata[103:96];
        if (RW0_wmask[13]) ram13[RW0_addr] <= RW0_wdata[111:104];
        if (RW0_wmask[14]) ram14[RW0_addr] <= RW0_wdata[119:112];
        if (RW0_wmask[15]) ram15[RW0_addr] <= RW0_wdata[127:120];
        if (RW0_wmask[16]) ram16[RW0_addr] <= RW0_wdata[135:128];
        if (RW0_wmask[17]) ram17[RW0_addr] <= RW0_wdata[143:136];
        if (RW0_wmask[18]) ram18[RW0_addr] <= RW0_wdata[151:144];
        if (RW0_wmask[19]) ram19[RW0_addr] <= RW0_wdata[159:152];
        if (RW0_wmask[20]) ram20[RW0_addr] <= RW0_wdata[167:160];
        if (RW0_wmask[21]) ram21[RW0_addr] <= RW0_wdata[175:168];
        if (RW0_wmask[22]) ram22[RW0_addr] <= RW0_wdata[183:176];
        if (RW0_wmask[23]) ram23[RW0_addr] <= RW0_wdata[191:184];
        if (RW0_wmask[24]) ram24[RW0_addr] <= RW0_wdata[199:192];
        if (RW0_wmask[25]) ram25[RW0_addr] <= RW0_wdata[207:200];
        if (RW0_wmask[26]) ram26[RW0_addr] <= RW0_wdata[215:208];
        if (RW0_wmask[27]) ram27[RW0_addr] <= RW0_wdata[223:216];
        if (RW0_wmask[28]) ram28[RW0_addr] <= RW0_wdata[231:224];
        if (RW0_wmask[29]) ram29[RW0_addr] <= RW0_wdata[239:232];
        if (RW0_wmask[30]) ram30[RW0_addr] <= RW0_wdata[247:240];
        if (RW0_wmask[31]) ram31[RW0_addr] <= RW0_wdata[255:248];
      end else begin
        reg_RW0_addr <= RW0_addr;
      end
    end
  end

endmodule

module tag_array_1_ext(
  input RW0_clk,
  input [5:0] RW0_addr,
  input RW0_en,
  input RW0_wmode,
  input [3:0] RW0_wmask,
  input [95:0] RW0_wdata,
  output [95:0] RW0_rdata
);

  reg [23:0] ram0 [63:0];
  reg [23:0] ram1 [63:0];
  reg [23:0] ram2 [63:0];
  reg [23:0] ram3 [63:0];
  reg [5:0] reg_RW0_addr;

  assign RW0_rdata[23:0] = ram0[reg_RW0_addr];
  assign RW0_rdata[47:24] = ram1[reg_RW0_addr];
  assign RW0_rdata[71:48] = ram2[reg_RW0_addr];
  assign RW0_rdata[95:72] = ram3[reg_RW0_addr];

  always @(posedge RW0_clk) begin
    if (RW0_en) begin
      if (RW0_wmode) begin
        if (RW0_wmask[0]) ram0[RW0_addr] <= RW0_wdata[23:0];
        if (RW0_wmask[1]) ram1[RW0_addr] <= RW0_wdata[47:24];
        if (RW0_wmask[2]) ram2[RW0_addr] <= RW0_wdata[71:48];
        if (RW0_wmask[3]) ram3[RW0_addr] <= RW0_wdata[95:72];
      end else begin
        reg_RW0_addr <= RW0_addr;
      end
    end
  end

endmodule

module tag_array_2_ext(
  input RW0_clk,
  input [5:0] RW0_addr,
  input RW0_en,
  input RW0_wmode,
  input [3:0] RW0_wmask,
  input [91:0] RW0_wdata,
  output [91:0] RW0_rdata
);

  reg [22:0] ram0 [63:0];
  reg [22:0] ram1 [63:0];
  reg [22:0] ram2 [63:0];
  reg [22:0] ram3 [63:0];
  reg [5:0] reg_RW0_addr;

  assign RW0_rdata[22:0] = ram0[reg_RW0_addr];
  assign RW0_rdata[45:23] = ram1[reg_RW0_addr];
  assign RW0_rdata[68:46] = ram2[reg_RW0_addr];
  assign RW0_rdata[91:69] = ram3[reg_RW0_addr];

  always @(posedge RW0_clk) begin
    if (RW0_en) begin
      if (RW0_wmode) begin
        if (RW0_wmask[0]) ram0[RW0_addr] <= RW0_wdata[22:0];
        if (RW0_wmask[1]) ram1[RW0_addr] <= RW0_wdata[45:23];
        if (RW0_wmask[2]) ram2[RW0_addr] <= RW0_wdata[68:46];
        if (RW0_wmask[3]) ram3[RW0_addr] <= RW0_wdata[91:69];
      end else begin
        reg_RW0_addr <= RW0_addr;
      end
    end
  end

endmodule

module data_arrays_0_2_ext(
  input RW0_clk,
  input [8:0] RW0_addr,
  input RW0_en,
  input RW0_wmode,
  input [3:0] RW0_wmask,
  input [127:0] RW0_wdata,
  output [127:0] RW0_rdata
);

  reg [31:0] ram0 [511:0];
  reg [31:0] ram1 [511:0];
  reg [31:0] ram2 [511:0];
  reg [31:0] ram3 [511:0];
  reg [8:0] reg_RW0_addr;

  assign RW0_rdata[31:0] = ram0[reg_RW0_addr];
  assign RW0_rdata[63:32] = ram1[reg_RW0_addr];
  assign RW0_rdata[95:64] = ram2[reg_RW0_addr];
  assign RW0_rdata[127:96] = ram3[reg_RW0_addr];

  always @(posedge RW0_clk) begin
    if (RW0_en) begin
      if (RW0_wmode) begin
        if (RW0_wmask[0]) ram0[RW0_addr] <= RW0_wdata[31:0];
        if (RW0_wmask[1]) ram1[RW0_addr] <= RW0_wdata[63:32];
        if (RW0_wmask[2]) ram2[RW0_addr] <= RW0_wdata[95:64];
        if (RW0_wmask[3]) ram3[RW0_addr] <= RW0_wdata[127:96];
      end else begin
        reg_RW0_addr <= RW0_addr;
      end
    end
  end

endmodule
