import re
import time
import BlynkLib
import threading
import subprocess

blynk = BlynkLib.Blynk('64vBgqBMh3kK7eEPWFgMblKib7sNwqLk');
cputemp, publicIP, localIP, upTime, nowTime = '', '', '', '', ''
isAutoFan = False

def updateCPUTemp() : # get CPU Temperature
    global cputemp, isAutoFan
    cputemp = subprocess.check_output('/opt/vc/bin/vcgencmd measure_temp | cut -d "=" -f 2 | cut -d "' + "'" + '" -f 1', shell=True).decode()[:-1]
    blynk.virtual_write(9, cputemp + '°C')

    if float(cputemp) > 57.0 :
		blynk.virtual_write(2, 1)

	elif float(cputemp) < 50.0 :
		blynk.virtual_write(2, 0)

    threading.Timer(3, updateCPUTemp).start()

def updateIP() : # get public / local IP address
    global publicIP
    try : publicIP = subprocess.check_output('/usr/bin/dig +short myip.opendns.com @resolver1.opendns.com', shell=True).decode()[:-1]
    except subprocess.CalledProcessError : publicIP = 'unknown'

    global localIP
    try : localIP = subprocess.check_output('ifconfig wlan0 | grep "inet " | cut -d " " -f 10-11', shell=True).decode()[:-2]
    except subprocess.CalledProcessError : localIP = 'unknown'

    blynk.virtual_write(6, publicIP + ' : ' + localIP)
    threading.Timer(3600, updateIP).start()

def updateServerUptime() : # get server uptime
    global upTime
    upTime = subprocess.check_output('uptime -p | cut -d " " -f2- | sed "s/hours/hrs/; s/minutes/min/; s/,//"', shell=True).decode()[:-1]

    blynk.virtual_write(7, upTime)
    threading.Timer(60, updateServerUptime).start()

def updateServerTime() : # get system time
    global nowTime
    nowTime = subprocess.check_output('date "+%F %T.%N" | rev | cut -c 7- | rev', shell=True).decode()[:-1]

    blynk.virtual_write(8, nowTime)
    threading.Timer(1, updateServerTime).start()

updateIP()
updateCPUTemp()
updateServerTime()
updateServerUptime()

while True :
    blynk.run()
