import serial
import logging
import subprocess
import os
import struct
from time import sleep

#Constant 
READ_TIMEOUT = 50
RX_PACKET_LEN = 38
RX_BUFFER_SIZE = 78
READ_TIMEOUT = 10
PACKET_HEAD = 85
PACKET_TAIL = 86
NOT_FOUND = -1

def findPacket(data):
    if len(data)>=RX_PACKET_LEN:
        for i in range(0,len(data)-RX_PACKET_LEN+1):
            logging.debug("Head: %d, TAIL: %d",data[i],data[i+RX_PACKET_LEN-1])
            if data[i] == PACKET_HEAD and data[i+RX_PACKET_LEN-1] == PACKET_TAIL:
                return i+1
    return NOT_FOUND

def main():
    logging.basicConfig(level=logging.INFO)

    name = input("Please enter the file name: ")
    relativePath = name+".csv"
    script_dir = "./"
    f = open(os.path.join(script_dir, relativePath),'w')
    logging.info("File opened.")
    
    ser = serial.Serial()
    ser.baudrate = 115200
    ser.timeout = READ_TIMEOUT
    # change the port name here
    ser.port = '/dev/cu.usbmodem146401'
    ser.open()
    sleep(0.05)
    logging.info("----------Serial port opened.--------")

    f.write("accelX,accelY,accelZ,gyroX,gyroY,gyroZ,magnetX,magnetY,magnetZ\n")

    accel = [0,0,0]
    altitude = 0
    r = 0
    index = -1
    while True:
        ser.reset_input_buffer()
        data = ser.read(RX_BUFFER_SIZE)
        index  = findPacket(data)
        if index != NOT_FOUND:
            accelX = (struct.unpack_from('f',data,index)[0])
            accelY = (struct.unpack_from('f',data,index+4)[0])
            accelZ = (struct.unpack_from('f',data,index+8)[0])
            gyroX = (struct.unpack_from('f',data,index+12)[0])
            gyroY = (struct.unpack_from('f',data,index+16)[0])
            gyroZ = (struct.unpack_from('f',data,index+20)[0])
            magnetX = (struct.unpack_from('f',data,index+24)[0])
            magnetY = (struct.unpack_from('f',data,index+28)[0])
            magnetZ = (struct.unpack_from('f',data,index+32)[0])
            logging.info("%.2f, %.2f, %.2f,%.2f, %.2f, %.2f,%.2f, %.2f, %.2f",accelX,accelY,accelZ,gyroX,gyroY,gyroZ,magnetX,magnetY,magnetZ)
            f.write(str(accelX)+","+str(accelY)+","+str(accelZ)+","+str(gyroX)+","+str(gyroY)+","+str(gyroZ)+","+str(magnetX)+","+str(magnetY)+","+str(magnetZ)+"\n")

if __name__ == '__main__':
    main()
