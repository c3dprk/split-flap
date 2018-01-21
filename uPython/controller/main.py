from machine import Pin, SPI
from splitflap import SplitFlap, luts
from time import sleep

OUT = Pin.OUT
IN = Pin.IN

NUM_COL = 22
NUM_ROW = 2

char_lut = luts.char_lut


]

#Hammi add
words = [
#                          |23
   
    "i am a display from   the space age",
    "oldschool tech is     awesome!",
    "Vorsprung durch       Technik",
    "Spass am Geraet.",
    "/(ØWØ)/",
    "War is Peace",
    "Freedom is Slavery",
    "Ignorance is Strength",
    "Vectoring is Glasfaser",
    "Made possible by hammi space and marble",
	"The only way to win the game is not to Play!"
	
#                          |23
	
]

#Hamm added Test
test = [
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
"BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
"CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC",
"DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD",
"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE",
"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
"",
]
#Hammi add end.
def vals_to_mat(vals, lut, w, h, default):
    vals = vals.upper()
    mat = [[default for a in range(h)] for b in range(w)]
    for y in range(h):
        for x in range(w):
            i = x + y*w
            if (i < len(vals)) and (vals[i] in lut):
                mat[x][y] = lut[vals[i]]
    return mat

def show_words():
    for word in words:
        sf.set(vals_to_mat(word, char_lut, NUM_COL, NUM_ROW, char_lut[' ']))
        sleep(1) # Abfrage
		
# Added from Hammi
def run_test():
    for word in test:
        sf.set(vals_to_mat(word, char_lut, NUM_COL, NUM_ROW, char_lut[' ']))
        sleep(1) # Abfrage
#End from Hammi

spi = SPI(-1, baudrate=int(500e3), phase=1, polarity=0, sck=Pin(5), miso=Pin(4), mosi=Pin(16))
col_rclk = Pin(12, OUT)
start_rclk = Pin(13, OUT)
face_load = Pin(15, OUT)

sf = SplitFlap(NUM_COL, NUM_ROW, spi, col_rclk, start_rclk, face_load)
sf.set(vals_to_mat("DEBUG IP: {}".format(WLAN().ifconfig()[0]), char_lut, NUM_COL, NUM_ROW, char_lut[' ']))

while(1):
    show_words()
