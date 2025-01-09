import sensor, image, time, lcd
from maix import KPU
import gc

lcd.init()
sensor.reset()                      # Reset and initialize the sensor. It will
                                    # run automatically, call sensor.run(0) to stop
sensor.set_pixformat(sensor.RGB565) # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)   # Set frame size to QVGA (320x240)
sensor.set_windowing((224, 224))
sensor.skip_frames(time = 1000)     # Wait for settings take effect.
clock = time.clock()                # Create a clock object to track the FPS.

kpu = KPU()
kpu.load_kmodel("/sd/KPU/mnist/uint8_mnist_cnn_model.kmodel")

while True:
    gc.collect()
    img = sensor.snapshot()
    img_mnist1=img.to_grayscale(1)        #convert to gray
    img_mnist2=img_mnist1.resize(112,112)
    img_mnist2.invert()                 #invert picture as mnist need
    img_mnist2.strech_char(1)           #preprocessing pictures, eliminate dark corner
    img_mnist2.pix_to_ai()

    out = kpu.run_with_output(img_mnist2, getlist=True)
    max_mnist = max(out)
    index_mnist = out.index(max_mnist)
    score = KPU.sigmoid(max_mnist)
    if index_mnist == 1: #白底
        if score > 0.999:
            display_str = "num: %d" % index_mnist
            print(display_str, score)
            img.draw_string(4,3,display_str,color=(0,0,0),scale=2)
    elif index_mnist == 5: #黑底
        if score > 0.999:
            display_str = "num: %d" % index_mnist
            print(display_str, score)
            img.draw_string(4,3,display_str,color=(0,0,0),scale=2)
    else:
        display_str = "num: %d" % index_mnist
        print(display_str, score)
        img.draw_string(4,3,display_str,color=(0,0,0),scale=2)
    lcd.display(img)

kpu.deinit()
