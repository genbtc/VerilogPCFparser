// blackiceII -> generic interface
`include "bsp/core-defines.v"

module soc_top (
   input  clk,
   input  greset,
   input  DONE,
   input  DBG1,
   output [17:0] ADR,
   inout  [15:0] DAT,
   output RAMOE,
   output RAMWE,
   output RAMCS,
   output RAMLB,
   output RAMUB,
   inout  [55:0] PMOD,
   input  B1,
   input  B2,
   input  QSPICSN,
   input  QSPICK,
   output [3:0] QSPIDQ
   );
   wire reset_;
   sync_reset u_sync_reset(
      .clk(clk),
      .reset_in_(!greset),
      .reset_out_(reset_)
   );

   /* verilator lint_off PINMISSING */

   wire arm_ss;
   wire arm_mosi;
   wire arm_miso;
   wire arm_sclk;
   wire spi_mux;

   wire led4;
   wire led3;
   wire led2;
   wire led1;

   SB_IO #( .PIN_TYPE(6'b 1010_01))
   arm_spi_pins [2:0] (
      .PACKAGE_PIN({PMOD[52], PMOD[54], PMOD[55]}),
      .OUTPUT_ENABLE(spi_mux),
      .D_OUT_0({led4, led2, led1}),
      .D_IN_0({arm_ss, arm_mosi, arm_sclk}));
   assign PMOD[53] = spi_mux ? arm_miso : led3;
   assign QSPIDQ[3:0] = {4{1'b0}};

   `ifdef PS2
      wire ps2_clk;
      wire ps2_data;
      SB_IO #(.PIN_TYPE(6'b0000_01), .PULLUP(1'b1))
      ps2_io [1:0] (
         .PACKAGE_PIN({PMOD[22], PMOD[20]}),
         .D_IN_0({ps2_clk, ps2_data}));
   `endif

   `ifdef VGA
      wire [7:0] red;
      wire [7:0] green;
      wire [7:0] blue;
      wire hsync;
      assign PMOD[36] = hsync;
      wire vsync;
      assign PMOD[37] = vsync;
      assign PMOD[35:32] = green[7:4];
      assign PMOD[43:40] = red[7:4];
      assign PMOD[47:44] = blue[7:4];
   `endif

   `ifdef EXTRAM
      wire [`CORE_EXTRAM_BITS-1:0] ext_RAMDin;
      wire [`CORE_EXTRAM_BITS-1:0] ext_RAMDout;
      SB_IO #( .PIN_TYPE(6'b 1010_01) )
      sram_data_pins [`CORE_EXTRAM_BITS-1:0] (
         .PACKAGE_PIN(DAT[`CORE_EXTRAM_BITS-1:0]),
         .OUTPUT_ENABLE(!RAMWE),
         .D_OUT_0(ext_RAMDin),
         .D_IN_0(ext_RAMDout));
   `endif

   emu u_emu(
      // Minimal IO
      .CLK(clk),       .reset_(reset_),     .button(B2),         .spi_mux(spi_mux),
      .arm_ss(arm_ss), .arm_sclk(arm_sclk), .arm_mosi(arm_mosi), .arm_miso(arm_miso),
      .led_2(led2),    .led_1(led1),        .led_4(led4),        .led_3(led3)

      `ifdef SD_CARD
         ,.sd_ss(PMOD[49]), .sd_sclk(PMOD[48]), .sd_mosi(PMOD[51]), .sd_miso(B1)
      `endif

      `ifdef PS2
         ,.ps2_clk(ps2_clk), .ps2_data(ps2_data)
      `endif

      `ifdef VGA
         ,.red(red), .green(green), .blue(blue),
          .hsync(hsync), .vsync(vsync)
      `endif

      `ifdef EXTRAM
         ,.ram_ADR(ADR),
          .ram_Din(ext_RAMDin), .ram_Dout(ext_RAMDout),
          .ram_OE(RAMOE), .ram_CS(RAMCS),
          .ram_WE(RAMWE), .ram_BE({RAMUB, RAMLB})
      `endif
   );

endmodule
