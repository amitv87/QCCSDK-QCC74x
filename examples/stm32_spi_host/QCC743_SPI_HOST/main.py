import socket
import threading
import time

tcp_data_received = 0
tcp_data_send = 0
udp_data_received = 0
udp_data_send = 0
udp_packets_received = 0
udp_tx_exit_flag = False

def tcp_server(host, port):
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind((host, port))
    server_socket.listen(1)
    print("TCP Server listening on {}:{}".format(host, port))
    while True:
        client_socket, addr = server_socket.accept()
        print("TCP Connection from", addr)
        client_data = {"socket": client_socket, "addr": addr, "sender_active": False}
        threading.Thread(target=handle_tcp_client, args=(client_data,)).start()

def handle_tcp_client(client_data):
    client_socket = client_data["socket"]
    addr = client_data["addr"]
    while True:
        data = client_socket.recv(2048)
        if not data:
            print(f"Recved none from {addr}, thread exited.")
            break
        global tcp_data_received, udp_tx_exit_flag
        tcp_data_received += len(data)
        if (data.count(b'R') >= 32) and not client_data["sender_active"]:
            for thread in threading.enumerate():
                if thread.name == "udp_sender":
                    print("Found existing udp_sender thread, waiting for it to finish...")
                    udp_tx_exit_flag = True
                    thread.join()
            threading.Thread(target=tcp_sender, args=(client_data,)).start()
            client_data["sender_active"] = True
            print(f'start tcp sender thread for client {addr}')
            break
    #client_socket.close()

def udp_server(host, port):
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server_socket.bind((host, port))
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    print("UDP Server listening on {}:{}".format(host, port))
    clients = {}
    while True:
        data, addr = server_socket.recvfrom(40960)
        global udp_data_received, udp_packets_received, udp_tx_exit_flag
        udp_data_received += len(data)
        udp_packets_received += 1
        if addr not in clients:
            clients[addr] = {"sender_active": False}
        if (data.count(b'R') >= 32) and not clients[addr]["sender_active"]:
            for thread in threading.enumerate():
                if thread.name == "udp_sender":
                    print("Found existing udp_sender thread, waiting for it to finish...")
                    udp_tx_exit_flag = True
                    thread.join()
            udp_tx_exit_flag = False
            threading.Thread(target=udp_sender, args=(addr, server_socket, clients), name="udp_sender").start()
            clients[addr]["sender_active"] = True
            print(f'start udp sender thread for client {addr}')
        #print(f"Received over UDP from {addr}: {data}")

def tcp_sender(client_data):
    client_socket = client_data["socket"]
    message = bytes([ord('x')] * 1536)
    global tcp_data_send
    while True:
        try:
            tcp_data_send += len(message)
            client_socket.sendall(message)
        except Exception as e:
            print(f'exit tcp sender')
            break

def udp_sender(addr, server_socket, clients):
    message = bytes([ord('x')] * 1536)
    global udp_data_send, udp_tx_exit_flag
    while not udp_tx_exit_flag:
        try: 
            udp_data_send += len(message)
            server_socket.sendto(message, addr)
            time.sleep(1.0/5000)
        except Exception as e:
            break
    print(f'exit udp sender')

def print_stats():
    while True:
        global tcp_data_received, udp_data_received, tcp_data_send, udp_data_send, udp_packets_received
        tcp_rx_bandwidth = (tcp_data_received * 8) / (1024 * 1024)  # Convert to Mbps
        udp_rx_bandwidth = (udp_data_received * 8) / (1024 * 1024)  # Convert to Mbps
        
        tcp_tx_bandwidth = (tcp_data_send * 8) / (1024 * 1024)  # Convert to Mbps
        udp_tx_bandwidth = (udp_data_send * 8) / (1024 * 1024)  # Convert to Mbps
        udp_packet_rate = udp_packets_received
        print(f"TCP_RX:{tcp_rx_bandwidth:.2f}Mbps TCP_TX:{tcp_tx_bandwidth:.2f}Mbps, UDP_RX:{udp_rx_bandwidth:.2f}Mbps UDP_TX: {udp_tx_bandwidth:.2f}Mbps")
        tcp_data_received = 0
        udp_data_received = 0
        tcp_data_send = 0
        udp_data_send = 0
        udp_packets_received = 0
        time.sleep(1)

if __name__ == "__main__":
    HOST = '0.0.0.0'
    TCP_PORT = 5001
    UDP_PORT = 5001
    threading.Thread(target=tcp_server, args=(HOST, TCP_PORT)).start()
    threading.Thread(target=udp_server, args=(HOST, UDP_PORT)).start()
    threading.Thread(target=print_stats).start()
