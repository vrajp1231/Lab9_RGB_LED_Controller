#include <stdio.h>
#include <sys/mman.h> 
#include <fcntl.h> 
#include <unistd.h>
#define _BSD_SOURCE

#define GPIO_DIP_OFFSET 2
#define GPIO_LED_OFFSET 0 
#define GPIO_RGB_OFFSET 0
#define DIPS_AND_LEDS_BASEADDR 0x41200000
#define RGB_LEDS_BASEADDR 0x41210000

// the below code uses a device called /dev/mem to get a pointer to a physical
// address.  We will use this pointer to read/write the custom peripheral
volatile unsigned int * get_a_pointer(unsigned int phys_addr)
{
	int mem_fd = open("/dev/mem", O_RDWR | O_SYNC); 
	void *map_base = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, phys_addr); 
	volatile unsigned int *radio_base = (volatile unsigned int *)map_base; 
	return (radio_base);
}

int main()
{
	volatile unsigned int *dipsandleds_ptr = get_a_pointer(DIPS_AND_LEDS_BASEADDR);	
	volatile unsigned int *rgb_leds_ptr = get_a_pointer(RGB_LEDS_BASEADDR);
	
	printf("\r\nVraj Patel - Lab 9 RGB LEDs\n\r");
	printf("Use switches SW2-SW0 to control RGB LED colors\n\r");
	
	unsigned int led_counter = 0;
	unsigned int loop_count = 0;
	
	while (1)
	{
		unsigned int switch_val = *(dipsandleds_ptr + GPIO_DIP_OFFSET);
		unsigned int color_bits = switch_val & 0x7; // Extract lower 3 bits (SW2-SW0)
		
		// Map switch values to RGB LED colors
		// Bits: B1 G1 R1 B0 G0 R0
		unsigned int rgb_value = 0;
		
		switch(color_bits) {
			case 0b000: // OFF
				rgb_value = 0b000000;
				break;
			case 0b001: // Red
				rgb_value = 0b001001;
				break;
			case 0b010: // Green
				rgb_value = 0b010010;
				break;
			case 0b011: // Red + Green (Yellow)
				rgb_value = 0b011011;
				break;
			case 0b100: // Blue
				rgb_value = 0b100100;
				break;
			case 0b101: // Red + Blue (Purple)
				rgb_value = 0b101101;
				break;
			case 0b110: // Blue + Green (Cyan)
				rgb_value = 0b110110;
				break;
			case 0b111: // Red + Blue + Green (White)
				rgb_value = 0b111111;
				break;
		}
		
		// Write to RGB LEDs
		*(rgb_leds_ptr + GPIO_RGB_OFFSET) = rgb_value;
		
		// Update counting LEDs every 4 loops (basically 2 Hz)
		if (loop_count >= 4) {
			led_counter = (led_counter + 1) & 0xF;
			*(dipsandleds_ptr + GPIO_LED_OFFSET) = led_counter;
			loop_count = 0;
		}
		
		// Print status
		printf("Switches: %d, RGB Value: 0x%02X, LED Counter: %d\r\n", 
		       color_bits, rgb_value, led_counter);
		
		usleep(250000);
		loop_count++;
	}
	
	return 0;
}