#!/usr/bin/python

##PikesPeak SQL password for wxstation PW3EdmBSwefnJRE9
VERSION="0.1"

"""
MicroWXStation Project - LogSerial2MySQL Python Script v0.1
Logs the serial (USB) data output from the Arduino to the 
MySQL DB for accessing the WXData over the internet.

(C) 2013, Tyler H. Jones - me@tylerjones.me - https://tylerjones.me
"""

import time, shelve, os, sys, random, logs2mconfig 
import string, MySQLdb, threading, socket, shutil, serial

pid = str(os.getpid())
pidfile = "/tmp/logs2m.pid"

if os.path.isfile(pidfile):
    print "%s already exists, exiting" % pidfile
    sys.exit()
else:
    file(pidfile, 'w').write(pid)

BAUD = 9600 #Set Serial port Baud rate

## Validate config file
config_response = logs2mconfig.configure()
if config_response != "Error":
    print "*** Config read OK ***"
    conf = shelve.open(".config")
    for i,v in config_response.iteritems():
        conf[i] = v
    conf.close()
else:
    os.unlink(pidfile)
    sys.exit()

## Make the config dictionary
config = shelve.open(".config")

###############################################################################################
###############################################################################################
## Bot settings variables

MySQL_DB = config["MySQL_DB"]
MySQL_User = config["MySQL_User"]
MySQL_Pass = config["MySQL_Pass"]
MySQL_Host = config["MySQL_Host"]
MySQL_Port = config["MySQL_Port"]
MySQL_DBTable = config["MySQL_DBTable"]
SerialPort = config["SerialPort"]

ser = serial.Serial()
ser.port     = SerialPort
ser.baudrate = BAUD
ser.parity   = 'N'
ser.rtscts   = False
ser.xonxoff  = False
ser.timeout  = 1

## MySQL Class
class mysql:
    def __init__(self):
	self.running = True;
	try:
    	    self.sql = MySQLdb.connect(unix_socket = "/var/run/mysqld/mysqld.sock", user = MySQL_User, passwd = MySQL_Pass, db = MySQL_DB)
	except MySQLdb.Error, e:
	    self.running = True;
    	    print "*** MySQL Error %d: %s ***" % (e.args[0], e.args[1])
    	    print "*** FATAL: quitting ***"
            os.unlink(pidfile)
    	    sys.exit(1)
    
    def execute(self, query):
	self.cursor = self.sql.cursor()
	self.cursor.execute(query)
	self.open = True
	
    def rowcount(self):
	if self.open:
	    return self.cursor.rowcount
	else:
	    print "*** rowcount: No DB cursor is open! ***"
	    return 0

    def fetchone(self):
	if self.open:
	    return self.cursor.fetchone()
	else:
	    print "*** fetchone: No DB cursor is open! ***"
	    return 0

    def fetchall(self):
	if self.open:
	    return self.cursor.fetchall()
	else:
	    print "*** fetchall: No DB cursor is open! ***"
	    return 0

    def closecursor(self):
	if self.open:
	    self.sql.commit()
	    return self.cursor.close()
	else:
	    print "*** close: No DB cursor is open! ***"
	    return 0

    def close(self):
	if self.open:
	    self.sql.commit()
	    return self.cursor.close()
	else:
	    print "*** close: No DB cursor is open! ***"
	    return 0

###############################################################################################
###############################################################################################
## Functions

def dbVal(s): # Protect against MySQL injection attacks
    s = ''.join([ c for c in s if c not in ('\'','\x1a','\r','\n','\"','\x00', '\\')])
    return s

def WriteToDB(data):
    SQL = mysql()
    TempDHT = data[0]
    TempBMP = data[1]
    Humidity = data[2]
    Pressure = data[3]
    DewPoint = data[4]
    Altitude = data[5]
    
    SQL.execute("INSERT INTO " + MySQL_DBTable + " (timestamp, dht_temp, bmp_temp, humidity, pressure, dewpoint, altitude)  VALUES ('" + str(time.time()) + "','" + TempDHT + "','" + TempBMP + "','" + Humidity + "','" + Pressure + "','" + DewPoint + "','" + Altitude + "')")
    SQL.closecursor()
    SQL.close()
    

def ParseData(data):
    if "[" in data and "]" in data:
  	data = data.split("[")[1].split("]")[0]
	print "Bracket parsed data: " + data
	if len(data.split("|")) == 6:
	    WriteToDB(data.split("|"))
        else:
	    print "Not enough arguments in serial data!"
    else:
        print "Invalid data received! was not wrapped with '[ ]'!"
	

def readSerial():
    while True: 
        print "Attempting to read from serial port..."
        time.sleep(120)    
	data = ser.readline()
	print "Data Received: " + data
	ParseData(data.strip("\n").strip("\r"))


if __name__ == '__main__':

    try:
        try:
            ser.open()
        except serial.SerialException, e:
            sys.stderr.write("Could not open serial port %s: %s\n" % (ser.portstr, e))
            os.unlink(pidfile)
            sys.exit(1)
        readSerial();
    except (KeyboardInterrupt, SystemExit): # Wait for a keyboard interupt
        print "\n*** Received keyboard interrupt, quitting threads ***"
        os.unlink(pidfile)
        sys.exit(0)

