/**
 * @file  examples/fpga-blinky/fpga_blinky.v
 * @brief FPGA blinky Application
 *        
 *        Toggles D3 every 400ms. Driven by the internal HF
 *        oscillator. If you edit this example, be sure to
 *        rebuild the FPGA binary before building the nRF
 *        project. Run:
 *           "make build-verilog" 
 *        from this folder.
 * 
 * @attention Copyright 2021 Silicon Witchery AB
 *
 * Permission to use, copy, modify, and/or distribute this 
 * software for any purpose with or without fee is hereby
 * granted, provided that the above copyright notice and this
 * permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO 
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, 
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER 
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN 
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, 
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

module top (
            output wire D3,  // LED on S1 popout
           );


    // Counts the high frequency clock and lets us divide down
	reg [20:0] counter;

    // Counter which is sent to the pins. The last bit initiates low power mode
    reg [8:0] port;

    // Configure internal HS oscillator as 6MHz
	SB_HFOSC #(.CLKHF_DIV("0b11"))  osc(.CLKHFEN(1'b1),     // enable
  										.CLKHFPU(1'b1), // power up
  										.CLKHF(clk)         // output to sysclk
                                        ) /* synthesis ROUTE_THROUGH_FABRIC=0 */;

    // 100mS tick tock
	always @(posedge clk) begin

        // Count and increment the output port
        counter <= counter + 1'b1;
        if(counter == 20'd600000) begin
            port <= port + 1'b1;
            counter <= 0;
        end
	end

    assign D3 = port[2]; // LED should toggle every 400mS

endmodule