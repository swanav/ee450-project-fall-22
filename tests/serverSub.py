import socket

categories = [ ("CourseCode", 80), ("Credits", 81), ("Professor", 82), ("Days", 83), ("CourseName", 84) ]
ee_courses = ["EE450", "EE658", "EE604", "EE608", "EE520", "EE503", "EE510"]
cs_courses = ["CS100", "CS310", "CS561", "CS435", "CS356", "CS402", "CS570"]

cs_port = 22053
ee_port = 23053
main_server_port = 24053

sock = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
sock.bind(("127.0.0.1", main_server_port))

def send_to_server(port: int, message: bytes):
    destination = ("127.0.0.1", port)
    sock.sendto(message, destination)
    msg, address = sock.recvfrom(1024)
    print("Query result {}:{} {}".format(address[0], address[1], msg))
    assert len(msg) > 0

def test_cs_server():
    for i in range(0, len(cs_courses)):
        query = bytes([33, len(cs_courses[i]), *str.encode(cs_courses[i])])
        send_to_server(cs_port, query)
        for j in range(0, len(categories)):
            query = bytes([32, categories[j][1], *str.encode(cs_courses[i])])
            send_to_server(cs_port, query)

def test_ee_server():
    for i in range(0, len(ee_courses)):
        query = bytes([33, len(ee_courses[i]), *str.encode(ee_courses[i])])
        send_to_server(ee_port, query)
        for j in range(0, len(categories)):
            query = bytes([32, categories[j][1], *str.encode(ee_courses[i])])
            send_to_server(ee_port, query)


# test_cs_server()
# test_ee_server()
