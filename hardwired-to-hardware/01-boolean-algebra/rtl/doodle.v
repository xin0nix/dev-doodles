module doodle (
    input [1:0] reg_in,

    output and_out,
    output or_out,
    output xor_out
);

  assign and_out = reg_in[1] & reg_in[0];
  assign or_out  = reg_in[1] | reg_in[0];
  assign xor_out = reg_in[1] ^ reg_in[0];

endmodule
