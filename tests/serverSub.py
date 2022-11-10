import socket

valid_credentials = ["EE450,Days"]
invalid_credentials = [""]

ee_port = 22053
cs_port = 23053

# Create a UDP socket at client side
sock = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

for i in range(0, len(valid_credentials)):
    sock.sendto(str.encode(valid_credentials[i]), ("127.0.0.1", ee_port))
    msg, address = sock.recvfrom(1024)
    print("Query result {}:{} {}".format(address[0], address[1], msg))
