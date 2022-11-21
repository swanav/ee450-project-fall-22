import random
import socket

# categories = [ ("CourseCode", 80), ("Credits", 81), ("Professor", 82), ("Days", 83), ("CourseName", 84) ]
# ee_courses = ["EE450", "EE658", "EE604", "EE608", "EE520", "EE503", "EE510"]
# cs_courses = ["CS100", "CS310", "CS561", "CS435", "CS356", "CS402", "CS570"]

credentials = [ 
    # ("james","2kAnsa7s"), 
    # ("brittany","*Tri8tium9"), 
    # ("roger","cOntra4@positiv1e"), 
    # ("jonathan","4wI0(lful"), 
    # ("amber","Th!r3oe4s"),
    ("fvmxxerc","*Xvm2xmyq3"),
]

# udp_port = 24053
tcp_port = 25053

# udp_sock = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
# udp_sock.bind(("127.0.0.1", udp_port))

REQ_ID_AUTHENTICATE = 0x00

def connect_to_server(port: int) -> socket.socket:
    tcp_sock = socket.socket(family=socket.AF_INET, type=socket.SOCK_STREAM)
    tcp_sock.connect(("127.0.0.1", port))
    return tcp_sock

def authenticate(tcp: socket.socket, username: str, password: str) -> bool:
    print("Authenticating...")
    print(username)
    print(password)
    tcp.send(bytes([REQ_ID_AUTHENTICATE, len(username), len(password), *str.encode(username), *str.encode(password)]))
    # tcp.send(bytes([REQ_ID_AUTHENTICATE, len(username), len(password), *str.encode(username), *str.encode(password)]))
    # tcp.send(bytes([REQ_ID_AUTHENTICATE, len(username), len(password), *str.encode(username), *str.encode(password)]))

def send_to_server(port: int, message: bytes):
    pass
    
def main():
    tcp = connect_to_server(tcp_port)
    (user, passwd) = credentials[random.randrange(0, len(credentials))]
    authenticate(tcp, user, passwd)


if __name__ == "__main__":
    main()