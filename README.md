USB serial monitor
==================

uscmon is serial port sniffer for USB serial converters, like FTDI. For now, only FTDI is supported.

Following events are supported:

- data transfers
- baud rate changing
- parity, data, stop bits changing
- flow control changing
- DTR/DSR, RTS/CTS, RI events
- break, overrun/parity/frame error events

Build
=====

    $ qmake uscmon.pro
    $ make

To clean up:

    $ make clean
    $ make distclean

Setup
=====

Make sure, debug_fs is enabled and usbmon module is loaded. Read usbmon info for more information.

    # modprobe usbmon
    # mount | grep debugfs
    debugfs on /sys/kernel/debug type debugfs (rw,relatime)
    # ls -l /sys/kernel/debug/usb/usbmon/
    <should be non-empty>

Usage
=====

    sudo ./uscmon

For uscmon to detect the USB device, it must be re-plugged while uscmon is running.
A successful detection should look like this:

    $ sudo ./uscmon
    USB serial converter monitor
    new device found: 14:7 0403:6001 FT232R USB UART, FTDI, serial: AL0249S3

A debug session output of uscmon might look like this:

    Parity: NONE, 8 data bits,  1 stop bits
    DTR ON
    RTS ON
    Flow control: software
    Baudrate set: 115200
    Baudrate set: 1200
    RTS ON
    DTR OFF
    Parity: NONE, 8 data bits,  1 stop bits
    DTR OFF
    Flow control: software
    Baudrate set: 1200
    RTS ON
    DTR ON
    Parity: NONE, 8 data bits,  1 stop bits
    DTR ON
    Flow control: software
    DTR ON
    403:6001 Data out (7)
    07 42 a0 00 05 40 d2
    403:6001 Data out (7)
    07 42 a0 00 05 40 d2
    403:6001 Data out (7)
    07 42 a0 00 05 40 d2
    DTR OFF
    DTR OFF
    RTS OFF
    Parity: NONE, 8 data bits,  1 stop bits
    [...]
