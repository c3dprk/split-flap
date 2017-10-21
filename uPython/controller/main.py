from machine import Pin, SPI
from splitflap import SplitFlap, luts
from time import sleep

OUT = Pin.OUT
IN = Pin.IN

NUM_COL = 8
NUM_ROW = 1

char_lut = luts.char_lut

words = [
    "HELLO",
    "WORLD",
    ":D XD ;3",
    "/(ØWØ)/",
]

def vals_to_mat(vals, lut, w, h, default):
    mat = [[default for a in range(h)] for b in range(w)]
    for y in range(h):
        for x in range(w):
            i = x + y*w
            if (i < len(vals)) and (vals[i] in lut):
                mat[x][y] = lut[vals[i]]
    return mat

def test():
    for word in words:
        sf.set(vals_to_mat(word, char_lut, NUM_COL, NUM_ROW, char_lut[' ']))
        sleep(2)

spi = SPI(-1, baudrate=int(500e3), phase=1, polarity=0, sck=Pin(5), miso=Pin(4), mosi=Pin(16))
col_rclk = Pin(12, OUT)
start_rclk = Pin(13, OUT)
face_load = Pin(15, OUT)

sf = SplitFlap(NUM_COL, NUM_ROW, spi, col_rclk, start_rclk, face_load)
