from fpioa_manager import fm
from machine import UART
import time

# binding UART2 IO:6->RX, 8->TX
fm.register(6, fm.fpioa.UART2_RX)
fm.register(8, fm.fpioa.UART2_TX)

yb_uart = UART(UART.UART2, 115200, 8, 0, 0, timeout=1000, read_buf_len=4096)


write_bytes = b'hello yahboom\r\n'  #串口输出的数据  Data output from the serial port
last_time = time.ticks_ms()

try:
    while True:
        # send data per 2000ms
        if time.ticks_ms() - last_time > 2000:
            last_time = time.ticks_ms()
            yb_uart.write(write_bytes)
        # read and print data
        if yb_uart.any():
            read_data = yb_uart.read()
            yb_uart.write(b'read_data = '+read_data+'\r\n')  #uart2 serial port
            if read_data:
                print("read_data = ", read_data)             #CH340 serial port
except:
    pass

yb_uart.deinit()
del yb_uart
