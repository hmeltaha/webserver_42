# import socket
# import threading

# HOST = "127.0.0.1"
# PORT = 8081
# NUM_CLIENTS = 100

# def connect_client(i):
# 	try:
# 		s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# 		s.connect((HOST, PORT))
# 		print(f"[{i}] connected")
# 		# keep connection open briefly
# 		s.send(b"hello\n")
# 	except Exception as e:
# 		print(f"[{i}] error: {e}")
# 	finally:
# 		s.close()

# threads = []

# for i in range(NUM_CLIENTS):
# 	t = threading.Thread(target=connect_client, args=(i,))
# 	t.start()
# 	threads.append(t)

# for t in threads:
# 	t.join()

# print("done")


import socket, time

s = socket.socket()
s.connect(('127.0.0.1', 8080))
s.send(b'GET / HTTP/1.1\r\nHost: localhost\r\n')
for i in range(30):
    s.send(b'X-Slow: a\r\n')
    time.sleep(1)
    print(f"Sent header {i}, server still held...")