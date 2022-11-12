import socket

PORT = 21053

valid_credentials = ["neqiw,6oErwe1w)", "fvmxxerc,*Xvm2xmyq3", "vskiv,gSrxve8@tswmxmz5i", "nsrexler,8aM4(pjyp", "eqfiv,Xl!v7si8w"]
invalid_credentials = ["neqiw,sdfsdfsdf)", "sdf,*asfd asg", "vskiv,gSrxve8@sgfsg ", "sdfsd,dfgsds(pjyp", "eqfiv,Xl!asd"]
destination = ("127.0.0.1", PORT)

def attempt_auth_request(user: str, password: str, invalid_type: bool = False) -> bytes:
    sock = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
    buffer = bytes([1 if invalid_type else 0, len(user),len(password), *str.encode(user), *str.encode(password)])
    sock.sendto(buffer, destination)
    msg, address = sock.recvfrom(1024)
    print("Authentication result {}:{} {} 0x{}".format(address[0], address[1], msg.hex() == '0031', msg.hex()))
    return msg

def test_auth_request_success():
    response = attempt_auth_request("neqiw", "6oErwe1w)")
    assert(response.hex() == '0031')

def test_auth_request_invalid_user():
    response = attempt_auth_request("sdf", "*asfd asg")
    assert(response.hex() == '0032')

def test_auth_request_invalid_password():
    response = attempt_auth_request("neqiw", "*asfd asg")
    assert(response.hex() == '0033')

def test_auth_request_invalid_type():
    response = attempt_auth_request("neqiw", "6oErwe1w)", True)
    assert(response.hex() == '01')
