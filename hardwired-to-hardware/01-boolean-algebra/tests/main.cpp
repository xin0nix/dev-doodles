#include "Vdoodle.h"
#include <iostream>
#include <verilated_vcd_c.h>

int main() {
  Verilated::traceEverOn(true);
  VerilatedVcdC trace;
  Vdoodle top;
  top.trace(&trace, 5);
  trace.open("wave.vcd");
  struct _on_exit {
    VerilatedVcdC &t;
    ~_on_exit() { t.close(); }
  } onExit(trace);
  for (int i = 0; i < 4; ++i) {
    top.reg_in = i;
    top.eval();
    trace.dump(i);
    std::cout << i << "and=" << (int)top.and_out << ", or=" << (int)top.or_out
              << ", xor=" << (int)top.xor_out << '\n';
  }
}
