/*
 * Copyright (C) 2018 bzt (bztsrc@github)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include "uart.h"
void loadcode(unsigned long addr, unsigned long size);
void main()
{
    unsigned long size=0;
    unsigned long addr=0;
    extern void *_code;
    extern void *_end;
    extern unsigned long __loader_size;


    // set up serial console
    uart_init();

    // say hello. To reduce loader size I removed uart_puts()
again:
    uart_send('R');
    uart_send('B');
    uart_send('I');
    uart_send('N');
    uart_send('6');
    uart_send('4');
    uart_send('\r');
    uart_send('\n');
    // notify raspbootcom to send the kernel
    uart_send(3);
    uart_send(3);
    uart_send(3);

    addr=uart_getc();
    addr|=uart_getc()<<8;
    addr|=uart_getc()<<16;
    addr|=uart_getc()<<24;
    
    // read the kernel's size b *0x7fadc
    size=uart_getc();
    size|=uart_getc()<<8;
    size|=uart_getc()<<16;
    size|=uart_getc()<<24;

    if(size<64) {
        // size error
        uart_send('S');
        uart_send('E');
        goto again;
    }

    unsigned long end_addr = addr + size;
    int check = 0;

    if((end_addr <= (unsigned long)&_end) && (end_addr >= (unsigned long)&_code))
    {
        check = 1;
    }
    if((addr <= (unsigned long)&_end) && (addr >= (unsigned long)&_code))
    {
        check = 1;
    }
    if((addr <= (unsigned long)&_code) && (end_addr >= (unsigned long)&_end))
    {
        check = 1;
    }

    void (*ptr)(unsigned long, unsigned long);
    if(check)
    {
        char *newloader=(char*)0x20000;
        char *oldloader=(char*)&_code;
        unsigned long loadersize=(unsigned long)&_end - (unsigned long)&_code;
        while(loadersize--) *newloader++ = *oldloader++;
        ptr = (((unsigned long)loadcode - (unsigned long)&_code) + ((unsigned long)0x20000) );
    }
    else
    {
        ptr = loadcode;
    }
    
    (*ptr)(addr, size);
    
    //uart_init();
    

    // send negative or positive acknowledge
    

    // read the kernel
    
}

void loadcode(unsigned long addr, unsigned long size)
{
    uart_send('O');
    uart_send('K');
    char *kernel=(char*)addr;
    while(size--) *kernel++ = uart_getc();

    // restore arguments and jump to the new kernel.
    asm volatile (
        "mov x0, x10;"
        "mov x1, x11;"
        "mov x2, x12;"
        "mov x3, x13;"
        // we must force an absolute address to branch to
        "mov x30, %0; ret"
    ::"r"(addr):);
}