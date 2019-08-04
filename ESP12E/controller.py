import re
import time
import serial
import threading
import subprocess

comm = serial.Serial('/dev/ttyS0', 9600)
cputemp, publicIP, localIP, upTime, nowTime = '', '', '', '', ''

def updateCPUTemp() : # get CPU Temperature
	global cputemp
	cputemp = subprocess.check_output('/opt/vc/bin/vcgencmd measure_temp | cut -d "=" -f 2 | cut -d "' + "'" + '" -f 1', shell=True).decode()[:-1]
	threading.Timer(3, updateCPUTemp).start()

def updatePublicIP() : # get public IP address
	global publicIP
	try : publicIP = subprocess.check_output('/usr/bin/dig +short myip.opendns.com @resolver1.opendns.com', shell=True).decode()[:-1]
	except subprocess.CalledProcessError : publicIP = 'unknown'
	threading.Timer(3600, updatePublicIP).start()

def updateLocalIP() : # get local IP address
	global localIP
	try : localIP = subprocess.check_output('ifconfig wlan0 | grep "inet " | cut -d " " -f 10-11', shell=True).decode()[:-2]
	except subprocess.CalledProcessError : localIP = 'unknown'
	threading.Timer(1800, updateLocalIP).start()

def updateServerUptime() : # get server uptime
	global upTime
	upTime = subprocess.check_output('uptime -p | cut -d " " -f2- | sed "s/hours/hrs/; s/minutes/min/; s/,//"', shell=True).decode()[:-1]
	threading.Timer(60, updateServerUptime).start()

def updateServerTime() : # get system time
	global nowTime
	nowTime = subprocess.check_output('date "+%F %T.%N" | rev | cut -c 7- | rev', shell=True).decode()[:-1]
	threading.Timer(1, updateServerTime).start()

def transmit() : # transmit to ESP
	payload = cputemp + '/' + publicIP + '/' + localIP + '/' + upTime + '/' + nowTime + '|'
	comm.write(payload.encode('utf-8'))
	#print(payload)
	threading.Timer(1, transmit).start()

updateCPUTemp()
updatePublicIP()
updateLocalIP()
updateServerUptime()
updateServerTime()
transmit()
