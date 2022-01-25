import socket
import re

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

    if flag == 1 :
        my_img += message

    if bool(re.search("start", clientMsg)):
        my_img=''
        flag = 1
        print('start')

    if flag == 2:
        f = open("pissangas.jpg", "wb")
        f.write(my_img)
        f.close()
        my_img=''
        flag = 0
        print('saved image')
   
    
    # Sending a reply to client
    #UDPServerSocket.sendto(bytesToSend, address)
