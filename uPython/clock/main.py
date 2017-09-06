from machine import Pin, RTC
from time import sleep, sleep_ms, sleep_us
from json import load as jload
from ntptime import settime

ADDR_DELAY_US = 100
GOTO_DELAY_MS = 20
HOUR_LUT = jload(open("hour_lut.json"))
MINUTE_LUT = jload(open("minute_lut.json"))
TIME_ZONE = +2

encoder_pins = [Pin(i, Pin.IN, Pin.PULL_UP) for i in [0, 2, 4, 5, 12, 13]]

run = Pin(16, Pin.OUT)
run.off()
adc_hour = Pin(14, Pin.OUT)
adc_hour.off()
adc_minute = Pin(15, Pin.OUT)
adc_minute.off()

try:
    settime()
except:
    pass
rtc = RTC()

def select(adc, adl):
    if None != adc:
        adc.on()
    if None != adl:
        adl.on()

def deselect(adc, adl):
    if None != adc:
        adc.off()
    if None != adl:
        adl.off()

def start(adc, adl):
    select(adc, adl)
    run.on()
    sleep_us(ADDR_DELAY_US)
    deselect(adc, adl)

def stop(adc, adl):
    select(adc, adl)
    run.off()
    sleep_us(ADDR_DELAY_US)
    deselect(adc, adl)

def get_pos(adc, adl):
    select(adc, adl)
    sleep_us(ADDR_DELAY_US)
    retval = 0
    for pin in encoder_pins:
        retval <<= 1
        retval += pin.value()
    deselect(adc, adl)
    return retval

def go_to(adc, adl, val):
    while get_pos(adc, adl) != val:
        start(adc, adl)
        sleep_ms(GOTO_DELAY_MS)
        stop(adc, adl)

def update_time():
    hour, minute = rtc.datetime()[4:6]
    hour = (hour+int(TIME_ZONE))%24
    if 0 == minute:
        try:
            settime()
        except:
            pass
    go_to(adc_hour, None, HOUR_LUT[hour])
    go_to(adc_minute, None, MINUTE_LUT[minute])

while True:
    update_time()
    sleep(5)
