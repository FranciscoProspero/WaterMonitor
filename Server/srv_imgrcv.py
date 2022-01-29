import socket
import re
import datetime

localIP     = "192.168.68.250"
localPort   = 20001
bufferSize  = 1024
msgFromServer= "Hello UDP Client"
bytesToSend = str.encode(msgFromServer)

# Create a datagram socket
UDPServerSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

# Bind to address and ip
UDPServerSocket.bind((localIP, localPort))
print("UDP server up and listening")
flag = 0
counter = 0
# Listen for incoming datagrams
while(True):
    bytesAddressPair = UDPServerSocket.recvfrom(bufferSize)
    message = bytesAddressPair[0]
    address = bytesAddressPair[1]
    clientMsg = "{}".format(message)
    clientIP  = "Client IPAddress:{}".format(address)
    #print(clientIP)
    #print(clientMsg)
    if bool(re.search("stop", clientMsg)):   
        flag = 2
        print('stop')
        now = datetime.datetime.now()
        date = now.strftime('%Y-%m-%d-%H-%M')
        new = f'{date}_{counter}'
        counter += 1
        print(new)

    if flag == 1 :
        my_img += bytearray(message)

    if bool(re.search("start", clientMsg)):
        my_img = bytearray(b'')
        flag = 1
        counter = 0
        numbers = re.findall(r'\d+', clientMsg)
        if numbers:
            counter = int(numbers[0])
        else :
            counter = 0
        print(f'start {counter}')


    if flag == 2:
        path = f'/disks/Elements/WaterMeter/{new}.jpg'
        f = open(path, "wb")
        f.write(my_img)
        f.close()
        my_img=''
        flag = 0
        print('saved image')
   
    
    # Sending a reply to client
    #UDPServerSocket.sendto(bytesToSend, address)
