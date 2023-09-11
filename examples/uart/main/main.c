#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "soc/periph_defs.h"
#include "soc/system_reg.h"
#include <dport_access.h>
#include "soc/uart_struct.h"
#include "esp_attr.h"
#include <string.h>
#include <soc/uart_reg.h>
#include <soc/gpio_reg.h>
#include <soc/gpio_periph.h>
#include <soc/soc_caps.h>
#include <soc/uart_periph.h>
#include <esp_rom_gpio.h>
#include <unistd.h>


// Define UART-related functions using FORCE_INLINE_ATTR
// (These functions are used to access UART hardware registers)

// Get the number of characters in the UART receive FIFO
FORCE_INLINE_ATTR uint32_t uart_ll_get_rxfifo_len(int uart_num)
{
    return (*(volatile uint32_t*)(UART_STATUS_REG(uart_num)) & (UART_RXFIFO_CNT));
}

// Get the number of characters in the UART transmit FIFO
FORCE_INLINE_ATTR uint32_t uart_ll_get_txfifo_len(int uart_num)
{
    return SOC_UART_FIFO_LEN - (*(volatile uint32_t*)(UART_STATUS_REG(uart_num)) & (UART_TXFIFO_CNT));
}

// Read data from the UART receive FIFO
FORCE_INLINE_ATTR void uart_ll_read_rxfifo(int uart_num, uint8_t *buf, uint32_t rd_len)
{
    for (int i = 0; i < (int)rd_len; i++) {
        buf[i] = READ_PERI_REG(UART_FIFO_REG(uart_num));
    }
}

// Write data to the UART transmit FIFO
FORCE_INLINE_ATTR void uart_ll_write_txfifo(int uart_num, const uint8_t *buf, uint32_t wr_len)
{
    for (int i = 0; i < (int)wr_len; i++) {
         WRITE_PERI_REG(UART_FIFO_AHB_REG(uart_num), buf[i]);
    }
}

// Function to read a character from the UART receive FIFO
unsigned char uart_getstr(int uart_num)
{
    unsigned char data;
    while (uart_ll_get_rxfifo_len(uart_num) == 0) {
        ;
    }
    uart_ll_read_rxfifo(uart_num, &data, 1);
    return data;
}

// Function to send a character over UART
void uart_putchar(int uart_num,int c)
{

    while (uart_ll_get_txfifo_len(uart_num) <= 126) {
        ;
    }
    uart_ll_write_txfifo(uart_num, (uint8_t *)&c, 1);
}


// Function to send a string over UART
void uart_send_str(int uart_num,char *c)
{
    while (*c != 0) {
        uart_putchar(uart_num,*c);
        c++;
    }
}


void app_main(void) 
{

    // Enable and reset UART1
    DPORT_SET_PERI_REG_MASK(SYSTEM_PERIP_CLK_EN0_REG, SYSTEM_UART1_CLK_EN);
    DPORT_CLEAR_PERI_REG_MASK(SYSTEM_PERIP_RST_EN0_REG, SYSTEM_UART1_RST);

    REG_SET_BIT(SYSTEM_PERIP_RST_EN0_REG, SYSTEM_UART1_RST);
    REG_CLR_BIT(SYSTEM_PERIP_RST_EN0_REG, SYSTEM_UART1_RST);
    REG_CLR_BIT(SYSTEM_PERIP_CLK_EN0_REG, SYSTEM_UART1_CLK_EN_M);
    REG_SET_BIT(SYSTEM_PERIP_CLK_EN0_REG, SYSTEM_UART1_CLK_EN_M);

    int uart_num =1;
    int baud = 115200;

    const int sclk_div = 1;
    uint32_t sclk_freq = XTAL_CLK_FREQ;
    uint32_t clk_div = ((sclk_freq) << 4) / baud;

    // Configure UART clock and baud rate
    // (Set UART_CLKDIV_REG, UART_CLK_CONF_REG, and UART_SCLK_DIV_NUM)
    *(volatile uint32_t*)(UART_CLK_CONF_REG(uart_num)) = (*(volatile uint32_t*)(UART_CLK_CONF_REG(uart_num))) & (~(BIT(22)));
    *(volatile uint32_t*)(UART_CLK_CONF_REG(uart_num)) = (*(volatile uint32_t*)(UART_CLK_CONF_REG(uart_num))) & (~(BIT(25)));
    *(volatile uint32_t*)(UART_CLK_CONF_REG(uart_num)) = (*(volatile uint32_t*)(UART_CLK_CONF_REG(uart_num))) & (~(BIT(24)));

    
    *(volatile uint32_t*)(UART_CLK_CONF_REG(uart_num)) = (*(volatile uint32_t*)(UART_CLK_CONF_REG(uart_num))) | (BIT(22));
    *(volatile uint32_t*)(UART_CLK_CONF_REG(uart_num)) = (*(volatile uint32_t*)(UART_CLK_CONF_REG(uart_num))) | (BIT(25));
    *(volatile uint32_t*)(UART_CLK_CONF_REG(uart_num)) = (*(volatile uint32_t*)(UART_CLK_CONF_REG(uart_num))) | (BIT(24));

    
    *(volatile uint32_t*)(UART_CLKDIV_REG(uart_num)) = (*(volatile uint32_t*)(UART_CLKDIV_REG(uart_num)) & ~(UART_CLKDIV)) | ((clk_div >> 4) & (UART_CLKDIV));
    *(volatile uint32_t*)(UART_CLKDIV_REG(uart_num)) = (*(volatile uint32_t*)(UART_CLKDIV_REG(uart_num)) & ~(UART_CLKDIV_FRAG)) | ((clk_div &  0xf) & (UART_CLKDIV_FRAG));

    *(volatile uint32_t*)(UART_CLK_CONF_REG(uart_num)) = (*(volatile uint32_t*)(UART_CLK_CONF_REG(uart_num)) & ~(UART_SCLK_DIV_NUM)) | ((sclk_div - 1) & (UART_SCLK_DIV_NUM));

    // Configure UART frame format (bit number, stop bit, and parity)
    // (Set UART_CONF0_REG)
    int bit_number = 3;
    int stop_bit_number = 1;
    int parity_en = 0;

    *(volatile uint32_t*)(UART_CONF0_REG(uart_num)) = (*(volatile uint32_t*)(UART_CONF0_REG(uart_num)) & ~(UART_BIT_NUM)) | ((bit_number) & (UART_BIT_NUM));
    *(volatile uint32_t*)(UART_CONF0_REG(uart_num)) = (*(volatile uint32_t*)(UART_CONF0_REG(uart_num)) & ~(UART_STOP_BIT_NUM)) | ((stop_bit_number) & (UART_STOP_BIT_NUM));
    *(volatile uint32_t*)(UART_CONF0_REG(uart_num)) = (*(volatile uint32_t*)(UART_CONF0_REG(uart_num)) & ~(UART_PARITY_EN)) | ((parity_en) & (UART_PARITY_EN));


    // Reset and configure FIFOs and timeouts
    // (Set UART_CONF0_REG, UART_CONF1_REG, UART_MEM_CONF_REG)
    *(volatile uint32_t*)(UART_CONF0_REG(uart_num)) = (*(volatile uint32_t*)(UART_CONF0_REG(uart_num)) & ~(UART_RXFIFO_RST)) | ((1) & (UART_RXFIFO_RST));
    *(volatile uint32_t*)(UART_CONF0_REG(uart_num)) = (*(volatile uint32_t*)(UART_CONF0_REG(uart_num)) & ~(UART_RXFIFO_RST)) | ((0) & (UART_RXFIFO_RST));
    *(volatile uint32_t*)(UART_CONF0_REG(uart_num)) = (*(volatile uint32_t*)(UART_CONF0_REG(uart_num)) & ~(UART_TXFIFO_RST)) | ((1) & (UART_TXFIFO_RST));
    *(volatile uint32_t*)(UART_CONF0_REG(uart_num)) = (*(volatile uint32_t*)(UART_CONF0_REG(uart_num)) & ~(UART_TXFIFO_RST)) | ((0) & (UART_TXFIFO_RST));

    *(volatile uint32_t*)(UART_CONF1_REG(uart_num)) = (*(volatile uint32_t*)(UART_CONF1_REG(uart_num)) & ~(UART_RXFIFO_FULL_THRHD)) | ((80) & (UART_TXFIFO_RST));
    *(volatile uint32_t*)(UART_CONF1_REG(uart_num)) = (*(volatile uint32_t*)(UART_CONF1_REG(uart_num)) & ~(UART_RX_TOUT_EN)) | ((1) & (UART_RX_TOUT_EN));

    *(volatile uint32_t*)(UART_MEM_CONF_REG(uart_num)) = (*(volatile uint32_t*)(UART_MEM_CONF_REG(uart_num)) & ~(UART_RX_TOUT_THRHD)) | ((20) & (UART_RX_TOUT_THRHD));

    *(volatile uint32_t*)(UART_INT_RAW_REG(uart_num)) = (*(volatile uint32_t*)(UART_INT_RAW_REG(uart_num)) & ~(UART_RXFIFO_FULL_INT_RAW)) | ((1) & (UART_RXFIFO_FULL_INT_RAW));
    *(volatile uint32_t*)(UART_INT_RAW_REG(uart_num)) = (*(volatile uint32_t*)(UART_INT_RAW_REG(uart_num)) & ~(UART_RXFIFO_TOUT_INT_RAW)) | ((1) & (UART_RXFIFO_TOUT_INT_RAW));


    int tx_io_num = 8;
    int rx_io_num = 3;
    
    int rts_io_num =-1;
    int cts_io_num =-1;

    // Configure GPIO pins for UART communication
    // (Set GPIO_PIN_MUX_REG, GPIO_FUNC0_OUT_SEL_CFG_REG, and others)

    /* In the following statements, if the io_num is negative, no need to configure anything. */
    if (tx_io_num >= 0 ) {
        REG_SET_FIELD(GPIO_PIN_MUX_REG[tx_io_num], MCU_SEL, PIN_FUNC_GPIO);
        (*(volatile uint32_t *)(0x60004000 + 0x8)) = ((0x1 << (tx_io_num - 32)));
        REG_WRITE(GPIO_FUNC0_OUT_SEL_CFG_REG + (tx_io_num * 4),UART_PERIPH_SIGNAL(uart_num, SOC_UART_TX_PIN_IDX));
    }

    if (rx_io_num >= 0 ) {
        REG_SET_FIELD(GPIO_PIN_MUX_REG[rx_io_num], MCU_SEL, PIN_FUNC_GPIO);
        REG_CLR_BIT(IO_MUX_GPIO0_REG + (rx_io_num * 4), FUN_PD);
        REG_SET_BIT(GPIO_PIN_MUX_REG[rx_io_num], FUN_PU);
        PIN_INPUT_ENABLE(IO_MUX_GPIO0_REG + (rx_io_num * 4));
        esp_rom_gpio_connect_in_signal(rx_io_num, UART_PERIPH_SIGNAL(uart_num, SOC_UART_RX_PIN_IDX), 0);
    }

    if (rts_io_num >= 0 ) {
        REG_SET_FIELD(GPIO_PIN_MUX_REG[rts_io_num], MCU_SEL, PIN_FUNC_GPIO);
        (*(volatile uint32_t *)(0x60004000 + 0x8)) = ((0x1 << (rts_io_num - 32)));
        REG_WRITE(GPIO_FUNC0_OUT_SEL_CFG_REG + (rts_io_num * 4),UART_PERIPH_SIGNAL(uart_num, SOC_UART_TX_PIN_IDX));
    }

    if (cts_io_num >= 0  ) {
        REG_SET_FIELD(GPIO_PIN_MUX_REG[cts_io_num], MCU_SEL, PIN_FUNC_GPIO);
        REG_CLR_BIT(IO_MUX_GPIO0_REG + (cts_io_num * 4), FUN_PD);
        REG_SET_BIT(GPIO_PIN_MUX_REG[cts_io_num], FUN_PU);
        PIN_INPUT_ENABLE(IO_MUX_GPIO0_REG + (rx_io_num * 4));
        esp_rom_gpio_connect_in_signal(cts_io_num, UART_PERIPH_SIGNAL(uart_num, SOC_UART_CTS_PIN_IDX), 0);
    }

    char *data = "Working";
    while (1) 
    {
        uart_send_str(uart_num,data);
        printf("Recv str: ");
        for (size_t i = 0; i < strlen(data); i++)
        {
            char c = uart_getstr(uart_num);
            printf("%c", c);
        }
        printf("\n");
        usleep(200000);
    }



}