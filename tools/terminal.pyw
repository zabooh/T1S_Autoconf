import serial
import serial.tools.list_ports
import threading
import subprocess
import tkinter as tk
from tkinter import scrolledtext
from tkinter import ttk
import os  # Import the 'os' module for running the other Python program
import time
import random
import re
import matplotlib.pyplot as plt
import datetime as dt
from datetime import datetime as nowt

#######################################################################################
# 
#   Initialize Variables
#
#######################################################################################

# Default COM Port Settings
default_com_port_A = 'COM9'
default_com_port_B = 'COM12'
default_com_port_C = 'COM13'
default_com_port_D = 'COM14'
baud_rate = 115200

com_port_A = None
com_port_B = None
com_port_C = None 
com_port_D = None

serial_A = None  # Serial object for COM Port 1
serial_B = None  # Serial object for COM Port 2
serial_C = None  # Serial object for COM Port 3
serial_D = None  # Serial object for COM Port 4

timer_duration = None
timer_expired = None
timer = None

com_port_to_variable = {}

Message_Items = {}

MaxNodes = None
result_list = None

Parse_Data = False
Parse_String = None
Parse_pattern = None
Parse_serial = None

# Set the working directory to the directory where the main program is located
working_directory = os.path.dirname(os.path.abspath(__file__))
os.chdir(working_directory)

#######################################################################################
# 
#   Functions
#
#######################################################################################

# Function to send user input to the COM Port with CR+LF
def send_to_com_port(ser, user_input):
    if user_input.strip().startswith(">"):
        user_input = user_input.strip()[1:]  # Remove ">" at the beginning
    user_input = user_input + "\r\n"  # Add CR+LF
    ser.write(user_input.encode())
    print(f"Sent to COM Port: {user_input}")  # Debug output in the console

# Function to establish the COM Port connection
def connect_to_com_ports():
    global serial_A, serial_B, serial_C, serial_D, com_ports
    global com_port_A, com_port_B, com_port_C, com_port_D
    global com_port_to_variable

    # Retrieve all available COM Ports again
    com_ports = list(serial.tools.list_ports.comports())
    
    for port_info in com_ports:
        if port_info.serial_number:
            com_port_serial_dict[port_info.device] = port_info.serial_number
    
    update_com_port_label_x()
    
    com_port_A = com_port_entry_A.get()
    com_port_B = com_port_entry_B.get()    
    com_port_C = com_port_entry_C.get()
    com_port_D = com_port_entry_D.get()    
    
    print(f"Connecting to COM Ports...")

    try:
        if com_port_A != '': serial_A = serial.Serial(com_port_A, baud_rate, timeout=0)
        if com_port_B != '': serial_B = serial.Serial(com_port_B, baud_rate, timeout=0)
        if com_port_C != '': serial_C = serial.Serial(com_port_C, baud_rate, timeout=0)
        if com_port_D != '': serial_D = serial.Serial(com_port_D, baud_rate, timeout=0)

        print(f"Connected to COM Ports {com_port_A}, {com_port_B}, {com_port_C}, and {com_port_D}.")

        com_port_to_variable[com_port_A] = False
        com_port_to_variable[com_port_B] = False
        com_port_to_variable[com_port_C] = False
        com_port_to_variable[com_port_D] = False

        # Create and start threads to read data from the COM Ports
        if com_port_A != '': com_reader_thread_A = threading.Thread(target=read_from_com_port, args=(serial_A, text_widget_A))
        if com_port_A != '': com_reader_thread_A.daemon = True
        if com_port_A != '': com_reader_thread_A.start()

        if com_port_B != '': com_reader_thread_B = threading.Thread(target=read_from_com_port, args=(serial_B, text_widget_B))
        if com_port_B != '': com_reader_thread_B.daemon = True
        if com_port_B != '': com_reader_thread_B.start()

        if com_port_C != '': com_reader_thread_C = threading.Thread(target=read_from_com_port, args=(serial_C, text_widget_C))
        if com_port_C != '': com_reader_thread_C.daemon = True
        if com_port_C != '': com_reader_thread_C.start()

        if com_port_D != '': com_reader_thread_D = threading.Thread(target=read_from_com_port, args=(serial_D, text_widget_D))
        if com_port_D != '': com_reader_thread_D.daemon = True
        if com_port_D != '': com_reader_thread_D.start()

        # Disable the COM Port input fields and Connect button after connection
        if com_port_A != '': com_port_entry_A.config(state=tk.DISABLED)
        if com_port_B != '': com_port_entry_B.config(state=tk.DISABLED)
        if com_port_C != '': com_port_entry_C.config(state=tk.DISABLED)
        if com_port_D != '': com_port_entry_D.config(state=tk.DISABLED)
        connect_button.config(state=tk.DISABLED)
        disconnect_button.config(state=tk.NORMAL)

    except serial.SerialException as e:
        print(f"Serial connection error: {e}")

# Function to disconnect from the COM Ports
def disconnect_from_com_ports():
    global serial_A, serial_B
    global com_port_A, com_port_B, com_port_C, com_port_D

    if serial_A is not None:
        serial_A.close()
    if serial_B is not None:
        serial_B.close()
    if serial_C is not None:
        serial_C.close()    
    if serial_D is not None:
        serial_D.close()

    # Enable the COM Port input fields and Connect button after disconnect
    if com_port_A != '': com_port_entry_A.config(state=tk.NORMAL)
    if com_port_B != '': com_port_entry_B.config(state=tk.NORMAL)
    if com_port_C != '': com_port_entry_C.config(state=tk.NORMAL)
    if com_port_D != '': com_port_entry_D.config(state=tk.NORMAL)
    connect_button.config(state=tk.NORMAL)
    disconnect_button.config(state=tk.DISABLED)

# Function to read data from the COM Port and update the GUI text field
def read_from_com_port(ser, text_widget):
    while True:
        try:
            data = ser.read(1)  # Read one character from the COM Port
            if data:
                # process_vt100_escape(text_widget, data.decode())  # Display data in the text field and process VT100 escape sequences
                text_widget.insert(tk.END, data,"green_on_black")
                text_widget.see(tk.END)  # Scroll to the end of the text field
        except serial.SerialException as e:
            print(f"Serial connection error: {e}")
            break





def check_for_keywords(data):
    global Message_Items
    for keyword in Message_Items:
        if keyword in data:
            return keyword
    return None


def process_received_data(ser,data):
    global com_port_to_variable
    global result_list
    global MaxNodes
    global Parse_Data
    global Parse_String
    global Parse_pattern
    global Parse_serial

    if Parse_serial == ser:
        found_keyword = check_for_keywords(data)
        if found_keyword:
            print(f"Found Keyword: {found_keyword} @ {ser}")
            com_port_to_variable[ser.name] = True
        
    if Parse_Data == True:
        if Parse_String in data:
            matches = re.findall(Parse_pattern, data)
            if matches:
                result = [int(match) for match in matches]
                result_str = result[0]                
                result_list.append((MaxNodes, int(result_str)))
                
                print(result_list)
            else:
                print("no match found")



def read_from_com_port(ser, text_widget):
    buffer = b""
    while True:
        try:
            data = ser.read(1) # Ein Zeichen vom COM-Port lesen
            if data:
                # Den gelesenen Zeichenwert zum Puffer hinzufügen
                buffer += data

                # Überprüfen, ob eine vollständige Zeilenendekennung vorhanden ist
                if b'\n' in buffer:
                    lines = buffer.split(b'\n')
                    buffer = lines[-1]  # Das letzte Element ist möglicherweise unvollständig
                    for line in lines[:-1]:
                        try:
                            decoded_line = line.decode('utf-8', errors='ignore')
                            # Eine vollständige Zeile verarbeiten
                            process_received_data(ser,decoded_line)
                            text_widget.insert(tk.END, decoded_line+"\n", "green_on_black")
                            text_widget.see(tk.END)
                        except UnicodeDecodeError:
                            # Ungültige Bytes ignorieren
                            pass

                # Den gelesenen Zeichenwert im Textwidget anzeigen
                #text_widget.insert(tk.END, data, "green_on_black")
                #text_widget.see(tk.END)  # Zum Ende des Textwidgets scrollen
        except serial.SerialException as e:
            print(f"Serial connection error: {e}")
            break        


def start_terminal_program():
    # Specify the path to the Python program "terminal.pyw" here
    terminal_program_path = ".\check_serial_tk.pyw"

    # Use the 'subprocess' module to run the other Python program
    subprocess.Popen(["pythonw", terminal_program_path])

def update_com_port_label_x():
   com_port_label_A.config(text=f"COM Port A: {com_port_serial_dict.get(com_port_entry_A.get(), 'Not available')}")    
   com_port_label_B.config(text=f"COM Port B: {com_port_serial_dict.get(com_port_entry_B.get(), 'Not available')}")
   com_port_label_C.config(text=f"COM Port C: {com_port_serial_dict.get(com_port_entry_C.get(), 'Not available')}")    
   com_port_label_D.config(text=f"COM Port D: {com_port_serial_dict.get(com_port_entry_D.get(), 'Not available')}")

def clear_text():
    text_widget_A.delete(1.0, tk.END)
    text_widget_B.delete(1.0, tk.END)    
    text_widget_C.delete(1.0, tk.END)    
    text_widget_D.delete(1.0, tk.END)    

# Function to send iperf server command to COM Port A
def send_iperf_server_A_func():
    send_to_com_port(serial_A, "iperf -u -s")

# Function to send iperf client command to COM Port B
def send_iperf_client_B_func():
    send_to_com_port(serial_B, "iperf -u -c 192.168.100.11")

# Function to send iperf client command to COM Port C
def send_iperf_client_C_func():
    send_to_com_port(serial_C, "iperf -u -c 192.168.100.11")

# Function to send iperf client command to COM Port D
def send_iperf_client_D_func():
    send_to_com_port(serial_D, "iperf -u -c 192.168.100.11")

# Function to send run command to COM Port A
def send_run_A_func():
    send_to_com_port(serial_A, "run")

# Function to send run command to COM Port B
def send_run_B_func():
    send_to_com_port(serial_B, "run")

# Function to send run command to COM Port C
def send_run_C_func():
    send_to_com_port(serial_C, "run")

# Function to send run command to COM Port D
def send_run_D_func():
    send_to_com_port(serial_D, "run")

# Function to send reset command to all boards
def send_reset_all_boards():
    send_to_com_port(serial_A, "reset")
    send_to_com_port(serial_B, "reset")
    send_to_com_port(serial_C, "reset")
    send_to_com_port(serial_D, "reset")

def send_ndr_all_boards():
    send_to_com_port(serial_A, "ndr")
    send_to_com_port(serial_B, "ndr")
    send_to_com_port(serial_C, "ndr")
    send_to_com_port(serial_D, "ndr")

# Function to send netinfo command to all boards
def send_netinfo_func():
    send_to_com_port(serial_A, "netinfo")
    send_to_com_port(serial_B, "netinfo")
    send_to_com_port(serial_C, "netinfo")
    send_to_com_port(serial_D, "netinfo")

# Function to send PHY reset command to COM Port A
def send_reset_phy_A_func():
    send_to_com_port(serial_A, "miim wdata 32768")
    send_to_com_port(serial_A, "miim write 0")

# Function to send PHY reset command to COM Port B
def send_reset_phy_B_func():
    send_to_com_port(serial_B, "miim wdata 32768")
    send_to_com_port(serial_B, "miim write 0")

# Function to send PHY reset command to COM Port C
def send_reset_phy_C_func():
    send_to_com_port(serial_C, "miim wdata 32768")
    send_to_com_port(serial_C, "miim write 0")

# Function to send PHY reset command to COM Port D
def send_reset_phy_D_func():
    send_to_com_port(serial_D, "miim wdata 32768")
    send_to_com_port(serial_D, "miim write 0")

def GetTimeStamp():
    current_time = dt.datetime.now()
    formated_current_time = current_time.strftime("%Y-%m-%d %H:%M:%S.%f")
    milliseconds = formated_current_time[-6:-3]
    formated_current_time = current_time.strftime("%Y-%m-%d %H:%M:%S")
    formated_current_time_ms = formated_current_time + "." + milliseconds
    return formated_current_time_ms

Message_Items = ["BC_TEST_STATE_COORDINATOR_WAIT_FOR_REQUEST"]

def wait_fr_com_port(ser,Message):
    global Message_Items
    global com_port_to_variable
    global Parse_serial

    Parse_serial = ser

    Message_Items = [Message]
    COM_Port = ser.name
    while com_port_to_variable[COM_Port] == False: 
        root.update()  
    for COM_Port in com_port_to_variable:
        com_port_to_variable[COM_Port] = False       
    Message_Items = ['No Message to be received']


def start_Test_1():
    global timer_expired
    global timer

    text_widget_A.insert(tk.END, GetTimeStamp() + " Test 1 started\n","red_on_white")
    text_widget_B.insert(tk.END, GetTimeStamp() + " Test 1 started\n","red_on_white")
    text_widget_C.insert(tk.END, GetTimeStamp() + " Test 1 started\n","red_on_white")
    text_widget_D.insert(tk.END, GetTimeStamp() + " Test 1 started\n","red_on_white")

    send_to_com_port(serial_A, "reset")
    send_to_com_port(serial_B, "reset")
    send_to_com_port(serial_C, "reset")
    send_to_com_port(serial_D, "reset")
    wait_fr_com_port(serial_A, "BC_TEST_STATE_IDLE")
    send_to_com_port(serial_A, "run")
    wait_fr_com_port(serial_A, "BC_TEST_STATE_COORDINATOR_WAIT_FOR_REQUEST")
    send_to_com_port(serial_B, "run")
    wait_fr_com_port(serial_B, "BC_TEST_STATE_IDLE")
    send_to_com_port(serial_C, "run")
    wait_fr_com_port(serial_C, "BC_TEST_STATE_IDLE")    
    send_to_com_port(serial_D, "run")
    wait_fr_com_port(serial_D, "BC_TEST_STATE_IDLE")
    send_to_com_port(serial_A, "ndr")  
    send_to_com_port(serial_B, "ndr")  
    send_to_com_port(serial_C, "ndr")  
    send_to_com_port(serial_D, "ndr")  
    send_to_com_port(serial_A, "netinfo")  
    send_to_com_port(serial_B, "netinfo")  
    send_to_com_port(serial_C, "netinfo")  
    send_to_com_port(serial_D, "netinfo")  
    
    text_widget_A.insert(tk.END, GetTimeStamp() + " Test 1 Ready\n","red_on_white")
    text_widget_B.insert(tk.END, GetTimeStamp() + " Test 1 Ready\n","red_on_white")
    text_widget_C.insert(tk.END, GetTimeStamp() + " Test 1 Ready\n","red_on_white")    
    text_widget_D.insert(tk.END, GetTimeStamp() + " Test 1 Ready\n","red_on_white")

    text_widget_A.see(tk.END) 
    text_widget_B.see(tk.END) 
    text_widget_C.see(tk.END) 
    text_widget_D.see(tk.END) 

    root.update()


def start_Test_1_2():
    global timer_expired
    global timer
    last_function_called = None  # Variable to store the last function called

    functions_to_call = [
        lambda: send_to_com_port(serial_B, "run"),
        lambda: send_to_com_port(serial_C, "run"),
        lambda: send_to_com_port(serial_D, "run")
    ]
    random.shuffle(functions_to_call)

    text_widget_A.insert(tk.END, GetTimeStamp() + " Test 1.2 started\n", "red_on_white")
    text_widget_B.insert(tk.END, GetTimeStamp() + " Test 1.2 started\n", "red_on_white")
    text_widget_C.insert(tk.END, GetTimeStamp() + " Test 1.2 started\n", "red_on_white")
    text_widget_D.insert(tk.END, GetTimeStamp() + " Test 1.2 started\n", "red_on_white")

    send_to_com_port(serial_A, "reset")
    send_to_com_port(serial_B, "reset")
    send_to_com_port(serial_C, "reset")
    send_to_com_port(serial_D, "reset")
    wait_fr_com_port(serial_A, "BC_TEST_STATE_IDLE")
    send_to_com_port(serial_A, "run")
    wait_fr_com_port(serial_A, "BC_TEST_STATE_COORDINATOR_WAIT_FOR_REQUEST")

    for function in functions_to_call:
        time.sleep(random.uniform(0, 3))  # Random delay between 0 and 3 seconds
        text_widget_A.insert(tk.END, GetTimeStamp() + "\n")
        function()
        last_function_called = function  # Update the last function called

    send_to_com_port(serial_A, "ndr")  
    send_to_com_port(serial_B, "ndr")  
    send_to_com_port(serial_C, "ndr")  
    send_to_com_port(serial_D, "ndr")  
    send_to_com_port(serial_A, "netinfo")  
    send_to_com_port(serial_B, "netinfo")  
    send_to_com_port(serial_C, "netinfo")  
    send_to_com_port(serial_D, "netinfo")  
    WaitTime(2)

    # Access the last function executed
    print("Last function called:", last_function_called)

    text_widget_A.insert(tk.END, GetTimeStamp() + " Test 1.2 Ready\n", "red_on_white")
    text_widget_B.insert(tk.END, GetTimeStamp() + " Test 1.2 Ready\n", "red_on_white")
    text_widget_C.insert(tk.END, GetTimeStamp() + " Test 1.2 Ready\n", "red_on_white")
    text_widget_D.insert(tk.END, GetTimeStamp() + " Test 1.2 Ready\n", "red_on_white")

    text_widget_A.see(tk.END)
    text_widget_B.see(tk.END)
    text_widget_C.see(tk.END)
    text_widget_D.see(tk.END)

    root.update()


def start_Test_2():
    global timer_expired
    global timer
    global MaxNodes
    global result_list
    global Parse_Data 
    global Parse_String
    global Parse_pattern
    global Parse_serial

    Parse_String = "[0.0- 1.9 sec]"
    Parse_pattern = r'(\d+) Kbps'
    Parse_serial = serial_A
    Parse_Data = True
    result_list = []

    text_widget_A.insert(tk.END, GetTimeStamp() + " Test 2 Started\n","red_on_white")
    text_widget_B.insert(tk.END, GetTimeStamp() + " Test 2 Started\n","red_on_white")
    text_widget_C.insert(tk.END, GetTimeStamp() + " Test 2 Started\n","red_on_white")
    text_widget_D.insert(tk.END, GetTimeStamp() + " Test 2 Started\n","red_on_white")

    for MaxNodes in [8, 16, 24, 32, 40, 48, 64, 96, 128, 255]:
        
        send_to_com_port(serial_A, "reset" + "\n")
        send_to_com_port(serial_B, "reset" + "\n")
        wait_fr_com_port(serial_A, "BC_TEST_STATE_IDLE")        
        send_to_com_port(serial_A, "nds 0 " + str(MaxNodes) + "\n")
        send_to_com_port(serial_B, "nds 1 " + str(MaxNodes) + "\n")
        send_to_com_port(serial_A, "run" + "\n")
        wait_fr_com_port(serial_A, "BC_TEST_STATE_COORDINATOR_WAIT_FOR_REQUEST")        
        send_to_com_port(serial_B, "run" + "\n")
        wait_fr_com_port(serial_B, "BC_TEST_STATE_IDLE")        
        send_to_com_port(serial_A, "iperf -u -s" + "\n")
        wait_fr_com_port(serial_A, "iperf: Server listening on UDP port 5001")
        send_to_com_port(serial_B, "iperf -u -c 192.168.100.11 -t 2" + "\n")
        wait_fr_com_port(serial_B, "iperf: instance 0 completed.")

    Parse_Data = False

    # Trennen Sie die Werte in separate Listen für x- und y-Koordinaten
    x_values = [item[0] for item in result_list]
    y_values = [item[1] for item in result_list]

    # Erstellen Sie ein Liniendiagramm
    plt.figure(figsize=(8, 6))  # Größe des Diagramms festlegen (optional)
    plt.plot(x_values, y_values, marker='o', linestyle='-')

    # Beschriftungen und Titel hinzufügen
    plt.xlabel('Max Nodes')
    plt.ylabel('Bandwidth Bits/secs')
    plt.title('iperf Bandwidth in Respect to Max Nodes')

    # Gitterlinien hinzufügen (optional)
    plt.grid(True)

    # Diagramm anzeigen
    plt.show()

    text_widget_A.insert(tk.END, GetTimeStamp() + " Test 2 Ready\n","red_on_white")
    text_widget_B.insert(tk.END, GetTimeStamp() + " Test 2 Ready\n","red_on_white")
    text_widget_C.insert(tk.END, GetTimeStamp() + " Test 2 Ready\n","red_on_white")
    text_widget_D.insert(tk.END, GetTimeStamp() + " Test 2 Ready\n","red_on_white")

    text_widget_A.see(tk.END) 
    text_widget_B.see(tk.END) 
    text_widget_C.see(tk.END) 
    text_widget_D.see(tk.END) 

    root.update()

def start_Test_2_1():
    global timer_expired
    global timer
    global MaxNodes
    global result_list
    global Parse_Data 
    global Parse_String
    global Parse_pattern
    global Parse_serial

    Parse_String = "[0.0- 1.9 sec]"
    Parse_pattern = r'(\d+) Kbps'
    Parse_serial = serial_A
    Parse_Data = True
    result_list = []

    text_widget_A.insert(tk.END, GetTimeStamp() + " Test 2 Started\n","red_on_white")
    text_widget_B.insert(tk.END, GetTimeStamp() + " Test 2 Started\n","red_on_white")
    text_widget_C.insert(tk.END, GetTimeStamp() + " Test 2 Started\n","red_on_white")
    text_widget_D.insert(tk.END, GetTimeStamp() + " Test 2 Started\n","red_on_white")

    for MaxNodes in [8, 128, 96, 64, 48, 40, 32, 24, 16, 8]:
        
        send_to_com_port(serial_A, "reset" + "\n")
        send_to_com_port(serial_B, "reset" + "\n")
        wait_fr_com_port(serial_A, "BC_TEST_STATE_IDLE")        
        send_to_com_port(serial_A, "nds 0 " + str(MaxNodes) + "\n")
        send_to_com_port(serial_B, "nds 1 " + str(MaxNodes) + "\n")
        send_to_com_port(serial_A, "run" + "\n")
        wait_fr_com_port(serial_A, "BC_TEST_STATE_COORDINATOR_WAIT_FOR_REQUEST")        
        send_to_com_port(serial_B, "run" + "\n")
        wait_fr_com_port(serial_B, "BC_TEST_STATE_IDLE")        
        send_to_com_port(serial_A, "iperf -u -s")
        wait_fr_com_port(serial_A, "iperf: Server listening on UDP port 5001")
        send_to_com_port(serial_B, "iperf -u -c 192.168.100.11 -t 2" + "\n")
        wait_fr_com_port(serial_B, "iperf: instance 0 completed.")

    Parse_Data = False

    # Trennen Sie die Werte in separate Listen für x- und y-Koordinaten
    x_values = [item[0] for item in result_list]
    y_values = [item[1] for item in result_list]

    # Erstellen Sie ein Liniendiagramm
    plt.figure(figsize=(8, 6))  # Größe des Diagramms festlegen (optional)
    plt.plot(x_values, y_values, marker='o', linestyle='-')

    # Beschriftungen und Titel hinzufügen
    plt.xlabel('Max Nodes')
    plt.ylabel('Bandwidth Bits/secs')
    plt.title('iperf Bandwidth in Respect to Max Nodes')

    # Gitterlinien hinzufügen (optional)
    plt.grid(True)

    # Diagramm anzeigen
    plt.show()

    text_widget_A.insert(tk.END, GetTimeStamp() + " Test 2 Ready\n","red_on_white")
    text_widget_B.insert(tk.END, GetTimeStamp() + " Test 2 Ready\n","red_on_white")
    text_widget_C.insert(tk.END, GetTimeStamp() + " Test 2 Ready\n","red_on_white")
    text_widget_D.insert(tk.END, GetTimeStamp() + " Test 2 Ready\n","red_on_white")

    text_widget_A.see(tk.END) 
    text_widget_B.see(tk.END) 
    text_widget_C.see(tk.END) 
    text_widget_D.see(tk.END) 

    root.update()

def start_Test_3():
    global timer_expired
    global timer
    global MaxNodes
    global result_list
    global Parse_Data 
    global Parse_String
    global Parse_pattern
    global Parse_serial

    Parse_String = "[0.0- 2.1 sec]"
    Parse_pattern = r'(\d+) Kbps'
    Parse_serial = serial_A
    Parse_Data = True
    result_list = []
    

    text_widget_A.insert(tk.END, GetTimeStamp() + " Test 3 Started\n","red_on_white")
    text_widget_B.insert(tk.END, GetTimeStamp() + " Test 3 Started\n","red_on_white")
    text_widget_C.insert(tk.END, GetTimeStamp() + " Test 3 Started\n","red_on_white")
    text_widget_D.insert(tk.END, GetTimeStamp() + " Test 3 Started\n","red_on_white")

    for MaxNodes in [8, 16, 24, 32, 40, 48, 64, 96, 128, 255]:
        
        send_to_com_port(serial_A, "reset")
        send_to_com_port(serial_B, "reset")
        send_to_com_port(serial_C, "reset")
        send_to_com_port(serial_D, "reset")
        wait_fr_com_port(serial_A, "BC_TEST_STATE_IDLE")        
        send_to_com_port(serial_A, "nds 0 " + str(MaxNodes))
        send_to_com_port(serial_B, "nds 1 " + str(MaxNodes))
        send_to_com_port(serial_C, "nds 2 " + str(MaxNodes))
        send_to_com_port(serial_D, "nds 3 " + str(MaxNodes))
        send_to_com_port(serial_A, "run")
        wait_fr_com_port(serial_A, "BC_TEST_STATE_COORDINATOR_WAIT_FOR_REQUEST")        
        send_to_com_port(serial_B, "run")
        wait_fr_com_port(serial_B, "BC_TEST_STATE_IDLE")
        send_to_com_port(serial_C, "run")
        wait_fr_com_port(serial_C, "BC_TEST_STATE_IDLE")    
        send_to_com_port(serial_D, "run")
        wait_fr_com_port(serial_D, "BC_TEST_STATE_IDLE")          
        send_to_com_port(serial_A, "iperf -u -s")
        wait_fr_com_port(serial_A, "iperf: Server listening on UDP port 5001")
        send_to_com_port(serial_B, "iperf -u -c 192.168.100.11 -t 2")
        WaitTime(3)
        send_to_com_port(serial_C, "iperf -u -c 192.168.100.11 -t 2")
        WaitTime(3)
        send_to_com_port(serial_D, "iperf -u -c 192.168.100.11 -t 2")
        wait_fr_com_port(serial_D, "iperf: instance 0 completed.")

    Parse_Data = False
    
    # Trennen Sie die Werte in separate Listen für x- und y-Koordinaten
    x_values = [item[0] for item in result_list]
    y_values = [item[1] for item in result_list]

    # Erstellen Sie ein Liniendiagramm
    plt.figure(figsize=(8, 6))  # Größe des Diagramms festlegen (optional)
    plt.plot(x_values, y_values, marker='o', linestyle='-')

    # Beschriftungen und Titel hinzufügen
    plt.xlabel('Max Nodes')
    plt.ylabel('Bandwidth Bits/secs')
    plt.title('iperf Bandwidth in Respect to Max Nodes')

    # Gitterlinien hinzufügen (optional)
    plt.grid(True)

    # Diagramm anzeigen
    plt.show()

    text_widget_A.insert(tk.END, GetTimeStamp() + " Test 2 Ready\n","red_on_white")
    text_widget_B.insert(tk.END, GetTimeStamp() + " Test 2 Ready\n","red_on_white")
    text_widget_C.insert(tk.END, GetTimeStamp() + " Test 2 Ready\n","red_on_white")
    text_widget_D.insert(tk.END, GetTimeStamp() + " Test 2 Ready\n","red_on_white")

    text_widget_A.see(tk.END) 
    text_widget_B.see(tk.END) 
    text_widget_C.see(tk.END) 
    text_widget_D.see(tk.END) 

    root.update()





def timer_callback():
    global timer_expired
    global timer
    timer_expired = True
    

def WaitTime(secs):
    global timer_expired
    timer_expired = False
    timer = threading.Timer(secs, timer_callback)
    timer.start()
    while not timer_expired:
        root.update()


# Funktion zum Speichern des Textinhalts in einer Datei
def save_text_to_file(widget, filename):
    text_content = widget.get("1.0", tk.END)
    content = text_content.replace("\r", "")
    with open(filename, "w") as file:
        file.write(content)

# Funktion zum Speichern des Inhalts aller Textfenster
def save_all_text():
    current_time = nowt.now().strftime("%Y-%m-%d_%H-%M-%S")

    # Speichern Sie den Inhalt jedes Textfensters in einer eigenen Datei
    save_text_to_file(text_widget_A, f"Log_A_{current_time}.txt")
    save_text_to_file(text_widget_B, f"Log_B_{current_time}.txt")
    save_text_to_file(text_widget_C, f"Log_C_{current_time}.txt")
    save_text_to_file(text_widget_D, f"Log_D_{current_time}.txt")

#######################################################################################
# 
#   Main
#
#######################################################################################

# Get COM Port information
com_ports = list(serial.tools.list_ports.comports())

# Create a dictionary for mapping serial numbers to COM Ports
com_port_serial_dict = {}
for port_info in com_ports:
    if port_info.serial_number:
        com_port_serial_dict[port_info.device] = port_info.serial_number

#############################################################################################        
# Create the GUI
root = tk.Tk()
root.title("COM Port GUI")

# Make the window resizable
root.geometry("1500x800")  # Starting window size
# root.attributes('-fullscreen', True)

# Frame for COM Port input fields and buttons
com_port_frame = tk.Frame(root)
com_port_frame.pack(pady=1, padx=1, fill=tk.X)

com_port_command = tk.Frame(root)
com_port_command.pack(pady=1, padx=1, fill=tk.X)

com_port_status = tk.Frame(root)
com_port_status.pack(pady=1, padx=1, fill=tk.X)

top_text_widgets_frame = tk.Frame(root)
top_text_widgets_frame.pack(side=tk.TOP, fill=tk.BOTH, expand=True)
#############################################################################################

#############################################################################################
# Connect Disconnect
connect_button = tk.Button(com_port_frame, text="Connect", command=connect_to_com_ports)
connect_button.pack(side=tk.LEFT)
disconnect_button = tk.Button(com_port_frame, text="Disconnect", command=disconnect_from_com_ports, state=tk.DISABLED)
disconnect_button.pack(side=tk.LEFT)
#############################################################################################

#############################################################################################
# COM Port A input field and button
com_port_label_A = tk.Label(com_port_frame, text="COM Port A:")
com_port_label_A.pack(side=tk.LEFT)
com_port_entry_A = tk.Entry(com_port_frame, width=8)
com_port_entry_A.insert(0, default_com_port_A)
com_port_entry_A.pack(side=tk.LEFT)

# COM Port B input field and button
com_port_label_B = tk.Label(com_port_frame, text="COM Port B:")
com_port_label_B.pack(side=tk.LEFT)
com_port_entry_B = tk.Entry(com_port_frame, width=8)
com_port_entry_B.insert(0, default_com_port_B)
com_port_entry_B.pack(side=tk.LEFT)

# COM Port C input field and button
com_port_label_C = tk.Label(com_port_frame, text="COM Port C:")
com_port_label_C.pack(side=tk.LEFT)
com_port_entry_C = tk.Entry(com_port_frame, width=8)
com_port_entry_C.insert(0, default_com_port_C)
com_port_entry_C.pack(side=tk.LEFT)

# COM Port D input field and button
com_port_label_D = tk.Label(com_port_frame, text="COM Port D:")
com_port_label_D.pack(side=tk.LEFT)
com_port_entry_D = tk.Entry(com_port_frame, width=8)
com_port_entry_D.insert(0, default_com_port_D)
com_port_entry_D.pack(side=tk.LEFT)
#############################################################################################

###################################################################################################
# Command Buttons
#
# Create a button to clear the text windows
clear_button_left = tk.Button(com_port_frame, text="Clear Windows", command=clear_text)
clear_button_left.pack(side=tk.LEFT)

# Create a button for iperf server
send_left_command = tk.Button(com_port_frame, text="Iperf Server A", command=send_iperf_server_A_func)
send_left_command.pack(side=tk.LEFT)

# Create a button for iperf client
send_right_command = tk.Button(com_port_frame, text="Iperf Client B", command=send_iperf_client_B_func)
send_right_command.pack(side=tk.LEFT)

send_right_command = tk.Button(com_port_frame, text="Iperf Client C", command=send_iperf_client_C_func)
send_right_command.pack(side=tk.LEFT)

send_right_command = tk.Button(com_port_frame, text="Iperf Client D", command=send_iperf_client_D_func)
send_right_command.pack(side=tk.LEFT)

send_netinfo = tk.Button(com_port_frame, text="netinfo", command=send_netinfo_func)
send_netinfo.pack(side=tk.LEFT)

send_rA = tk.Button(com_port_frame, text="run A", command=send_run_A_func)
send_rA.pack(side=tk.LEFT)

send_rB = tk.Button(com_port_frame, text="run B", command=send_run_B_func)
send_rB.pack(side=tk.LEFT)

send_rC = tk.Button(com_port_frame, text="run C", command=send_run_C_func)
send_rC.pack(side=tk.LEFT)

send_rD = tk.Button(com_port_frame, text="run D", command=send_run_D_func)
send_rD.pack(side=tk.LEFT)

send_reset_all_boards = tk.Button(com_port_frame, text="Reset All", command=send_reset_all_boards)
send_reset_all_boards.pack(side=tk.LEFT)

send_ndr_all_boards = tk.Button(com_port_frame, text="Show Nodes", command=send_ndr_all_boards)
send_ndr_all_boards.pack(side=tk.LEFT)

send_reset_phy_A_func_button = tk.Button(com_port_command, text="PHY Reset A", command=send_reset_phy_A_func)
send_reset_phy_A_func_button.pack(side=tk.LEFT)

send_reset_phy_B_func_button = tk.Button(com_port_command, text="PHY Reset B", command=send_reset_phy_B_func)
send_reset_phy_B_func_button.pack(side=tk.LEFT)

send_reset_phy_C_func_button = tk.Button(com_port_command, text="PHY Reset C", command=send_reset_phy_C_func)
send_reset_phy_C_func_button.pack(side=tk.LEFT)

send_reset_phy_D_func_button = tk.Button(com_port_command, text="PHY Reset D", command=send_reset_phy_D_func)
send_reset_phy_D_func_button.pack(side=tk.LEFT)



send_start_test_1_button = tk.Button(com_port_command, text="Test 1", command=start_Test_1)
send_start_test_1_button.pack(side=tk.LEFT)

send_start_test_1_2_button = tk.Button(com_port_command, text="Test 1.2", command=start_Test_1_2)
send_start_test_1_2_button.pack(side=tk.LEFT)

send_start_test_2_button = tk.Button(com_port_command, text="Test 2", command=start_Test_2)
send_start_test_2_button.pack(side=tk.LEFT)

send_start_test_2_1_button = tk.Button(com_port_command, text="Test 2.1", command=start_Test_2_1)
send_start_test_2_1_button.pack(side=tk.LEFT)

send_start_test_3_button = tk.Button(com_port_command, text="Test 3", command=start_Test_3)
send_start_test_3_button.pack(side=tk.LEFT)

save_button = tk.Button(com_port_command, text="Save", command=save_all_text)
save_button.pack(side=tk.LEFT)

###################################################################################################

###################################################################################################
# Button to start the Python program "terminal.pyw" in the upper right corner
start_button = tk.Button(com_port_frame, text="Show COM Ports", command=start_terminal_program)
start_button.pack(side=tk.LEFT)  # Use 'anchor="ne"' to place the button in the upper right corner
###################################################################################################

###################################################################################################
#
# Label for COM Port 1 and display serial number
com_port_label_A = tk.Label(com_port_status, text=f"COM Port A: {'?'}")
com_port_label_A.pack(side=tk.LEFT)

# Labels for other COM Ports
com_port_label_B = tk.Label(com_port_status, text=f"COM Port B: {'?'}")
com_port_label_B.pack(side=tk.LEFT)

com_port_label_C = tk.Label(com_port_status, text=f"COM Port C: {'?'}")
com_port_label_C.pack(side=tk.LEFT)

com_port_label_D = tk.Label(com_port_status, text=f"COM Port D: {'?'}")
com_port_label_D.pack(side=tk.LEFT)
###################################################################################################

###################################################################################################
# First text widget for COM3 output
text_widget_A = scrolledtext.ScrolledText(top_text_widgets_frame, width=40, height=15)
text_widget_A.tag_configure("green_on_black", foreground="light green", background="black", font=("Helvetica", 12, "bold"))
text_widget_A.tag_configure("red_on_white", foreground="red", background="white", font=("Helvetica", 12, "bold"))
text_widget_A.pack(side=tk.LEFT, padx=10, pady=10, fill=tk.BOTH, expand=True)

# Second text widget for COM4 output
text_widget_B = scrolledtext.ScrolledText(top_text_widgets_frame, width=40, height=15)
text_widget_B.tag_configure("green_on_black", foreground="light green", background="black", font=("Helvetica", 12, "bold"))
text_widget_B.tag_configure("red_on_white", foreground="red", background="white", font=("Helvetica", 12, "bold"))
text_widget_B.pack(side=tk.LEFT, padx=10, pady=10, fill=tk.BOTH, expand=True)

# Create a frame for the bottom two text widgets (COM5 and COM6)
bottom_text_widgets_frame = tk.Frame(root)
bottom_text_widgets_frame.pack(side=tk.TOP, fill=tk.BOTH, expand=True)

# Text widget for additional output (e.g., COM5)
text_widget_C = scrolledtext.ScrolledText(bottom_text_widgets_frame, width=40, height=15)
text_widget_C.tag_configure("green_on_black", foreground="light green", background="black", font=("Helvetica", 12, "bold"))
text_widget_C.tag_configure("red_on_white", foreground="red", background="white", font=("Helvetica", 12, "bold"))
text_widget_C.pack(side=tk.LEFT, padx=10, pady=10, fill=tk.BOTH, expand=True)

# Text widget for additional output (e.g., COM6)
text_widget_D = scrolledtext.ScrolledText(bottom_text_widgets_frame, width=40, height=15)
text_widget_D.tag_configure("green_on_black", foreground="light green", background="black", font=("Helvetica", 12, "bold"))
text_widget_D.tag_configure("red_on_white", foreground="red", background="white", font=("Helvetica", 12, "bold"))
text_widget_D.pack(side=tk.LEFT, padx=10, pady=10, fill=tk.BOTH, expand=True)
###################################################################################################

# Event binding for the text input field
text_widget_A.bind("<Return>", lambda event, text_widget=text_widget_A: send_to_com_port(serial_A, text_widget.get("insert linestart", "insert lineend")))
text_widget_B.bind("<Return>", lambda event, text_widget=text_widget_B: send_to_com_port(serial_B, text_widget.get("insert linestart", "insert lineend")))
text_widget_C.bind("<Return>", lambda event, text_widget=text_widget_C: send_to_com_port(serial_C, text_widget.get("insert linestart", "insert lineend")))
text_widget_D.bind("<Return>", lambda event, text_widget=text_widget_D: send_to_com_port(serial_D, text_widget.get("insert linestart", "insert lineend")))

# Start the Tkinter main loop
root.mainloop()
