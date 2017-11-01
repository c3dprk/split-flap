from time import sleep_ms
from math import ceil
from splitflap.utils.matrix import *

DELAY_MS = 0

class SplitFlap(object):
    def __init__(self, num_col, num_row, spi, col_rclk, start_rclk, face_load):
        self._num_col = num_col
        self._num_row = num_row
        self._bytes_col = ceil(num_col/8)
        self._bytes_row = ceil(num_row/8)
        self._spi = spi
        self._col_rclk = col_rclk
        self._start_rclk = start_rclk
        self._face_load = face_load

        self._col_rclk.off()
        self._start_rclk.off()
        self._face_load.on()

        #self.mat = [[1 for a in range(num_row)] for b in range(num_col)]

    ## Helpers
    # pulses a pin (this takes about 370us)
    @staticmethod
    def _pulse(pin):
        pin.value(not pin.value())
        pin.value(not pin.value())

    ## column selection functions

    def _select_col(self, col):
        self._spi.write((1<<col).to_bytes(self._bytes_col, 'big'))
        self._pulse(self._col_rclk)

    def _select_all_col(self):
        self._spi.write(bytes([0xff]*self._bytes_col))
        self._pulse(self._col_rclk)

    def _select_no_col(self):
        self._spi.write(bytes([0x00]*self._bytes_col))
        self._pulse(self._col_rclk)

    ## Start

    # starts every display according to the matrix
    def _start_false(self, mat):
        if (len(mat) < self._num_col) or (len(mat[0]) < self._num_row):
            print("Matrix to smol")
            return -1

        for col in range(self._num_col):
            start_byte = 0
            # contruct bytes to be shifted into start register
            for row in range(self._num_row-1, -1 ,-1):
                start_byte <<= 1
                if not mat[col][row]:
                    start_byte += 1
            # shift into start reg
            self._spi.write((start_byte).to_bytes(self._bytes_row, 'big'))
            # make visible
            self._pulse(self._start_rclk)
            # start
            self._select_col(col)
            self._select_no_col()

    # pos
    def _get_pos(self, col, row):
        self._select_col(col)
        self._pulse(self._face_load)
        result = [(b^0xff)>>1 for b in self._spi.read(self._num_row)][row]
        self._select_no_col()
        return result

    # return the two dimensional matrix of the current faces of all dislpays
    def _get_faces(self):
        # stop all splitflap displays
        self.stop_all()
        # iterate through each column
        result = []
        for col in range(self._num_col):
            self._select_col(col)
            self._pulse(self._face_load)
            result += [[(b^0xff)>>1 for b in self._spi.read(self._num_row)]]
            self._select_no_col()
        return result

    # public
    def start_all(self):
        self._start_false([[False]*self._num_row]*self._num_col)

    def stop_all(self):
        self._start_false([[True]*self._num_row]*self._num_col)

    def goto(self, col, row, face):
        while self._get_pos(col, row) != face:
            self.start_pos(col, row)
            sleep_ms(20)
            self.stop_pos(col, row)

    def set(self, mat):
        finished = True
        comp_mat = comp_mats(mat, self._get_faces())
        for col_vec in comp_mat:
            for row_val in col_vec:
                finished &= row_val
        while not finished:
            self._start_false(comp_mat)
            sleep_ms(DELAY_MS)
            self.stop_all()
            comp_mat = comp_mats(mat, self._get_faces())
            finished = True
            for col_vec in comp_mat:
                for row_val in col_vec:
                    finished &= row_val
