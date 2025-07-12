//deprecated

#define PORT 0x3f8          // COM1

extern unsigned char _inb(unsigned short port);
extern void _outb(unsigned short port, unsigned char data);

int init_serial() {
    _outb(PORT + 1, 0x00);    // Disable all interrupts
    _outb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    _outb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    _outb(PORT + 1, 0x00);    //                  (hi byte)
    _outb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    _outb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    _outb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
    _outb(PORT + 4, 0x1E);    // Set in loopback mode, test the serial chip
    _outb(PORT + 0, 0xAE);    // Test serial chip (send byte 0xAE and check if serial returns same byte)

    // Check if serial is faulty (i.e: not same byte as sent)
    if(_inb(PORT + 0) != 0xAE) {
        return 1;
    }

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    _outb(PORT + 4, 0x0F);
    return 0;
}

void write_serial(char data){
    while ((_inb(PORT + 5) & 0x20) == 0);
    _outb(PORT,data);
}

void print_serial(char* data){
    while (*(data+1)) write_serial(*data);
}
