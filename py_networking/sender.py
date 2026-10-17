import socket 

HOST = '127.0.0.1'
PORT = 5556

with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as client:
    message = "HELLO"
    print(f"Sending Message: {message}")

    client.sendto(message.encode('utf-8'), (HOST, PORT))




