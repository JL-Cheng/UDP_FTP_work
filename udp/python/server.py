import socket

size = 8192

count = 1
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('0.0.0.0', 9876))

try:
	while True:
		data, address = sock.recvfrom(size)
		data = str(count) + ' ' + bytes.decode(data)
		sock.sendto(str.encode(data.upper()), address)
		count = count + 1
finally:
	sock.close()
