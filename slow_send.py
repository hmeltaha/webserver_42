import socket
import time

s = socket.socket()
s.connect(("127.0.0.1", 8081))

msg = b"HELLO_WORLD_FROM_CLIENT_0123456789\r\n\r\n"

for c in msg:
	s.send(bytes([c]))
	time.sleep(0.2)

data = s.recv(1024)
print("Received response from server: ", data)
s.close()
