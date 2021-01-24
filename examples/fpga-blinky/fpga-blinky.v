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

module top (output wire D1,  // Pin
            output wire D2,  // Pin
            output wire D3,  // LED on S1 popout
            output wire D4,  // Pin
            output wire D5,  // Pin
            output wire D6,  // Pin
            output wire D7,  // Pin
            output wire D8,  // Pin
            input       SCK, // SPI clock
            input       SS,  // SPI select
            input       SI,  // SPI serial in
            output      SO   // SPI serial out
            );


    //-----------------------------------
    // SPI Loopback. Returns data sent
    // with one byte delay
    //-----------------------------------

    // 9 bit SPI buffer. 9th bit is the shifted out MSB
    reg [7:0] spi_buffer = 0;

    // Simply delay the SI data to SO by 8 cycles
	always @(posedge SCK) begin
        
        // We use an active high CS line
        if(SS) begin
            spi_buffer[0] <= SI; // MSB first
            spi_buffer[7:1] <= spi_buffer[6:0]; // Shift left
        end
    end

    assign SO = spi_buffer[7]; // Always output the MSB


    //-----------------------------------
    // Counter for testing IO. Cycles all
    // pins, and shuts down once done
    //-----------------------------------

    // Counts the high frequency clock and lets us divide down
	reg [20:0] counter;

    // Counter which is sent to the pins. The last bit initiates low power mode
    reg [8:0] port;

    // This bit will shut the oscillator down when port[8] goes high
    reg clock_en = 1;

    // Configure internal HS oscillator as 6MHz
	SB_HFOSC #(.CLKHF_DIV("0b11"))  osc(.CLKHFEN(1'b1),     // enable
  										.CLKHFPU(clock_en), // power up
  										.CLKHF(clk)         // output to sysclk
                                        ) /* synthesis ROUTE_THROUGH_FABRIC=0 */;



    // 100mS tick tock
	always @(posedge clk) begin

        // Count and increment the output port
        counter <= counter + 1'b1;
        if(counter == 20'd600000) begin
            port <= port + 1'b1;
            counter <= 0;

            // When ninth bit goes high
            if (port  == 9'd256) begin
                clock_en <= 0; // This will stop the counter
            end
        end
	end

   assign D1 = port[0];
   assign D2 = port[1];
   assign D3 = port[2]; // LED should blink every 400mS
   assign D4 = port[3];
   assign D5 = port[4];
   assign D6 = port[5];
   assign D7 = port[6];
   assign D8 = port[7];

endmodule