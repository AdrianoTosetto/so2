import socket
import time

HOST = "127.0.0.1"  # Standard loopback interface address (localhost)
PORT = 65432        # Port to listen on (non-privileged ports are > 1023)

SEND_DATA_TO_CLIENT = True

anchors_nmea = {
    'A': '$GPGGA,092750.0123,5321.6802,N,00630.3372,W,1,8,1.03,61.7,M,55.2,M,,*76\n',
    'B': '$GPGGA,666666,6666.555,S,44444.333,E,2,06,2.5,777.1,N,-74.2,N,,*86\n'
}


def send_data_to_client(conn, node_id):
    nmea = anchors_nmea[node_id]
    print('mandando mensagem nema para node ' + node_id + ' nmea = ' + nmea)
    
    conn.sendall(nmea.encode())
    time.sleep(10)

def parse(conn):
    data = conn.recv(1024)
    print("Message received: ", data.decode())
    node_id = data.decode()
    if SEND_DATA_TO_CLIENT:
        send_data_to_client(conn, node_id)



with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    while True:
        print(f"Waiting for connection in {HOST}:{PORT}")
        s.listen()
        conn, addr = s.accept()
        with conn:
            print("Connected by", addr)
            parse(conn)
            print("Connection Closed!")