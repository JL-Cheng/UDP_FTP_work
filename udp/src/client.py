import socket
 
size = 8192
 
try:
	sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	for num in range(0,51):
		sock.sendto(str.encode(str(num)), ('localhost', 9876))
		print (bytes.decode(sock.recv(size))+'\n')
	sock.close()
 
except:
	print ("cannot reach the server")
