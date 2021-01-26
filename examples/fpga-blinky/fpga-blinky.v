/*
    2020 Silicon Witchery (info@siliconwitchery.com)

    Licensed under a Creative Commons Attribution 
    4.0 International License. This code is provided
    as-is and no warrenty is given.

    Description:
     A simple test program to cycle all fpga pins
     though a 7 bit counter. When D4 is high the
     counter will run and pins will toggle.

     It uses the internal HF oscillator.
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