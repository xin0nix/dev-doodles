module doodle (
  input wire clk,
  input wire value,
  input wire enable,
  output reg storage
);

always @(posedge clk) begin
  if (enable) begin 
    storage <= value;
  end
end

endmodule

