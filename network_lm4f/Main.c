#define PART_LM4F120H5QR

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <RASLib/inc/common.h>
#include <RASLib/inc/gpio.h>
#include <RASLib/inc/time.h>
#include <driverlib/gpio.h>
#include <driverlib/uart.h>
#include <driverlib/sysctl.h>
#include <driverlib/pin_map.h>
#include <inc/hw_types.h>
#include <inc/hw_memmap.h>

#include <inc/lm4f120h5qr.h>

#define BUFF_LEN 256

void scan_message(char *buffer, int buff_length);

void init_UART1(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    /**
    *   Enable GPIO Pins for UART (PC4-5)
    */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    GPIOPinTypeUART(GPIO_PORTC_BASE, (GPIO_PIN_4 | GPIO_PIN_5));
    GPIOPinConfigure(GPIO_PC4_U1RX);
    GPIOPinConfigure(GPIO_PC5_U1TX);
    
    //GPIO_PORTC_PUR_R &= ~0x30;
    //GPIO_PORTC_PDR_R |= 0x30;
    
    /**
    *   Enable UART Functions
    */ 
    UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 115384,
        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
        UART_CONFIG_PAR_NONE));

    UARTEnable(UART1_BASE);
}

//char test[20] = "{msg: #\BS\t#}";
/**
*   Message Format
*   { - Start
*   } - End
*   # ... # denotes raw number
*/
void scan_message(char *buffer, int buff_length) {
    char tmp[20];
    char in_char;
    int i;
    int index = 0;
    int num_flag = 0;
    while(UARTCharGet(UART1_BASE) != '{') {}
    buffer[index++] = '{';
    do {
        in_char = UARTCharGet(UART1_BASE);
        
        if(num_flag && in_char != '#') {
            sprintf(tmp, "%d", (int) in_char);    // Incoming raw decimal, convert to ASCII
            for(i=0; i<strlen(tmp);i++) 
                buffer[index++] = tmp[i];          
        } else {
            buffer[index++] = in_char;
        }
        
        if(in_char == '#' && !num_flag) 
            num_flag = 1;
        else if(in_char == '#')                    // Closing '#'
            num_flag = 0;   
        
    } while(in_char != '}');
    buffer[index++] = '}';
    buffer[index] = '\0';
}

int main(void) {
    char data[BUFF_LEN];
    init_UART1();
    Printf("FPGA Log\n");
    
    while(1) {
        //Printf("%c ", UARTCharGet(UART1_BASE));
        scan_message(data, BUFF_LEN);
        Printf("%s\n",data);
        //Printf("%s\n", UARTCharGet(UART1_BASE));
    }
    return 0;
}
