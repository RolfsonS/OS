import socket

# Define the IP address and port to listen on
# Use "0.0.0.0" to listen on all available network interfaces
UDP_IP = "127.0.0.1" 
UDP_PORT = 5555

# 1. Create a UDP socket
# AF_INET = IPv4, SOCK_DGRAM = UDP
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# 2. Bind the socket to the IP and port
sock.bind((UDP_IP, UDP_PORT))

print(f"UDP server listening on {UDP_IP}:{UDP_PORT}...")

# 3. Enter a loop to continuously receive packets
try:
    while True:
        # Buffer size of 1024 bytes (adjust based on expected payload sizes)
        data, addr = sock.recvfrom(1024)
        
        # Decode bytes payload to string
        message = data.decode('utf-8', errors='replace')
        
        print(f"Received packet from {addr[0]}:{addr[1]}")
        print(f"Message: {message}\n")
        
        # Optional: Send a reply back to the sender
        # sock.sendto(b"ACK", addr)

except KeyboardInterrupt:
    print("\nServer shutting down.")
finally:
    # 4. Clean up and close the socket
    sock.close()
