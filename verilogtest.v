verilog 
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
   inout  [55:5] PMOD,
   input  B1,
   input  B2,
   input  QSPICSN,
   input  QSPICK,
   output [3:0] QSPIDQ #are comments possible?
);