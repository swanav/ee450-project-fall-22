import socket

valid_credentials = ["neqiw,6oErwe1w)", "fvmxxerc,*Xvm2xmyq3", "vskiv,gSrxve8@tswmxmz5i", "nsrexler,8aM4(pjyp", "eqfiv,Xl!v7si8w"]
invalid_credentials = ["neqiw,sdfsdfsdf)", "sdf,*asfd asg", "vskiv,gSrxve8@sgfsg ", "sdfsd,dfgsds(pjyp", "eqfiv,Xl!asd"]

port = 21053

# Create a UDP socket at client side
sock = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

user = "neqiw"
password = "6oErwe1w)"

buffer = bytes([0, len(user),len(password), *str.encode(user), *str.encode(password)])
sock.sendto(buffer, ("127.0.0.1", port))
msg, address = sock.recvfrom(1024)
print("Authentication result {}:{} {} 0x{}".format(address[0], address[1], msg.hex() == '0031', msg.hex()))

user = "sdf"
password = "*asfd asg"
buffer = bytes([0, len(user),len(password), *str.encode(user), *str.encode(password)])
sock.sendto(buffer, ("127.0.0.1", port))
msg, address = sock.recvfrom(1024)
print("Authentication result {}:{} {} 0x{}".format(address[0], address[1], msg.hex() == '0032', msg.hex()))

user = "neqiw"
password = "*asfd asg"
buffer = bytes([0, len(user),len(password), *str.encode(user), *str.encode(password)])
sock.sendto(buffer, ("127.0.0.1", port))
msg, address = sock.recvfrom(1024)
print("Authentication result {}:{} {} 0x{}".format(address[0], address[1], msg.hex() == '0033', msg.hex()))

buffer = bytes([1, len(user),len(password), *str.encode(user), *str.encode(password)])
sock.sendto(buffer, ("127.0.0.1", port))
msg, address = sock.recvfrom(1024)
print("Authentication result {}:{} {} 0x{}".format(address[0], address[1], msg.hex() == '01', msg.hex()))



# for i in range(0, len(valid_credentials)):
#     sock.sendto(str.encode(valid_credentials[i]), ("127.0.0.1", port))
#     msg, address = sock.recvfrom(1024)
#     print("Authentication result {}:{} {}".format(address[0], address[1], msg))

# for i in range(0, len(invalid_credentials)):
#     sock.sendto(str.encode(invalid_credentials[i]), ("127.0.0.1", port))
#     msg, address = sock.recvfrom(1024)
#     print("Authentication result {}:{} {}".format(address[0], address[1], msg))
