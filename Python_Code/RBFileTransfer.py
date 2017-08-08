#!/bin/python
#Arduino RockBlock Control Program
#Owned by Faisal Alshaya and developed by Jousha Heaney (2017)
#This is an opened source and free software
#Usage: " + sys.argv[0] + " [USB/Serial port] [RB Name]
#Last updated 14Jun2017
import serial, time, os, math, sys

#Globals
RB_MSG_SIZE = 60
MAX_FILE_SIZE = 3000
FILE_SEG_SIZE = 1000
#FILE_SEG_SIZE = RB_MSG_SIZE


def printMenu ():
	NUMSEL = 4
	print "\nRockBLOCK File Transmitter Menu"
	print "CAUTION: This implementation supports Send Only and Receive Only operations.\n\tMessages received while in Send Mode will be lost."
	print "\t1. Warmup Test\n\t2. Send a File\n\t3. Receive a File\n\t4. Quit"
	selection = raw_input("Enter selection number: ")
	if not selection.isdigit():
		selection = -1
		print "Invalid Selection"
	else:
		selection = int(selection)
	if (selection < 1) or (selection > NUMSEL):
		selection = -1
		print "Invalid Selection"
	return selection
	
def printFromSerial(numReads):
	for i in range(numReads):
		data = arduino.readline()
		while not data:
			data = arduino.readline()
		print "A: " + data[:-2].replace('\n','\\n').replace('\r','\\r')

def readUntil(s, content, echo = True):
	x = ""
	start = True
	while True:
		if start:
			sys.stdout.write("A:  ")
			start = False
		y = s.read()
		while not y:
			y = s.read()
		x += y
		if echo:
			sys.stdout.write(y)
		if y == '\n':
			sys.stdout.write("A:  ")
		if x.endswith(content):
			sys.stdout.write('\n')
			return x

def transferFileToArduino(filename):
	with open(filename, 'r') as infile:
		content = infile.read()
	content = "<file:" + filename + ">" + content #add filename to content
	content = content.replace('\n','\\n').replace('\r','\\r')#replace \n and \r whitespace in file before sending
	
	filesize = len(content)
	segments = int(math.ceil(float(filesize)/FILE_SEG_SIZE))
	print "P:  File: " + filename + "\tSize: " + str(filesize) + "\tRB Segments: " + str(segments)
	
	arduino.write(str(filesize))# sends filezise to arduino
	arduino.write(str(segments))# sends number of segments for the file
	arduino.flush() # force send of serial output 
	time.sleep(1)
	
	#send file
	for i in range(segments):
		line = content[i*FILE_SEG_SIZE:i*FILE_SEG_SIZE+FILE_SEG_SIZE] # send segment size chuck to arduino
		print "P:  Sending segment: " + str(i)
		
		arduino.write(line)
		time.sleep(1)
	readUntil(arduino,"File Transfer Complete\r\n")
	
###Main execution begins here###

#check to make sure a usb/serial port was provided
if len(sys.argv) < 3:
	print "Error: provide USB/Serial communication port"
	print "Usage: " + sys.argv[0] + " [USB/Serial port] [RB Name]"
	print "Example: " + sys.argv[0] + " COM5 RB0010757"
	quit();

#ID Number of connected RockBLOCK	
ROCKBLOCK_NAME = sys.argv[2]
	
#Try/Catch block for serial connection to Arduino
#if connection unavailable or lost, program will exit
	
try:
	#connect to arduino on serial
	#First argument to this program should be the string of the usb/serial port name
	arduino = serial.Serial(sys.argv[1], 9600, timeout=.1)
	time.sleep(1) #give the connection a second to settle
	
	#Execution Loop
	while True:
		#Get user input
		userIn = printMenu()
		#Operation from user selection
		if userIn == 1:
			#Clear serial buffers
			arduino.flushInput()
			arduino.flushOutput()

			#Send RB setup commands to Arduino
			arduino.write("RB_Warmup")
			console = readUntil(arduino,"Warmup complete\r\n")

		elif userIn == 2:
			filename = raw_input("Enter file name: ")
			#Add error checking
			#if file doesn't exist: break
			
			#filename = "Test.txt"
			#filename = "expShort.rtf"
			
			arduino.write("RB_Send")
			readUntil(arduino,"Entering File Send Mode\r\n")
			transferFileToArduino(filename)
			targetRockblock = raw_input("Enter target Rockblock number [RB00xxxxx] or press enter for server: ")
			if targetRockblock == "" or targetRockblock.find("RB00") < 0:
				targetRockblock = "None"
			arduino.write(targetRockblock)
			print "P:  Target RB: " + targetRockblock
			
			readUntil(arduino,"##Send complete##")
			#readUntil(arduino,"Test Done")
			
		elif userIn == 3:
			print "Receive File"
			filename = "Recv.txt"
			#if file exists ask if overwrite okay
				
			arduino.write("RB_Recv")
			#readUntil(arduino,"##Receive complete##")
			console = ""
			console = readUntil(arduino,"##Receive complete##").splitlines()
			#console = str(console)
			receivedFile = ""
			for line in console:
				if line.find("RB_Message: ") >=0:
					print "In Line: " + line
					line = line[21:]#deletes RB_Message & RB Name (below replaces as backup)
					line.replace("RB_Message: ","")#remove "RB_Message: "
					line.replace(ROCKBLOCK_NAME,"")#remove RB name from text
					line.replace("RBmsgComplete","")#remove completion message from text
					print "Out Line: " + line
					receivedFile += line
			#print "Message Received: " + receivedFile
			#Filename located between "<file: >"
			if receivedFile.find("<file:") >= 0:
				fStart = receivedFile.find("<file:") + 5 #find the start point of the phrase "<file:" 
				fEnd = receivedFile.find(">", fStart) #find the ">" after filename
				filename = receivedFile[fStart+1:fEnd] #filename is what's inside
				receivedFile = receivedFile[7+len(filename):] #remove the filename from the message content
				receivedFile.replace("<file:" + filename + ">","") #backup for the above line
				print "Filename:" + filename
			#Output received message to file
			with open("Received/"+filename, 'w') as outfile:
				receivedFile.replace('\\n','\n').replace('\\r','\r')#reverse subsitution used in send
				print "Outputing: " + receivedFile
				outfile.write(receivedFile)
			
			
		elif userIn == 4:
			quit()
except serial.SerialException:
	print "\nNo connection to Arduino"
	quit()




