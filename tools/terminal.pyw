import serial
import serial.tools.list_ports
import threading
import subprocess
import tkinter as tk
from tkinter import scrolledtext
import os  # Import the 'os' module for running the other Python program

#######################################################################################
# 
#   Initialize Variables
#
#######################################################################################

# Default COM Port Settings
default_com_port_A = 'COM3'
default_com_port_B = 'COM4'
default_com_port_C = 'COM14'
default_com_port_D = 'COM21'
baud_rate = 115200

com_port_A = None
com_port_B = None
com_port_C = None 
com_port_D = None

serial_A = None  # Serial object for COM Port 1
serial_B = None  # Serial object for COM Port 2
serial_C = None  # Serial object for COM Port 3
serial_D = None  # Serial object for COM Port 4

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

# Function for processing VT100 escape sequences
def process_vt100_escape(text_widget, data):
    # VT100 escape sequences can be processed here
    # Example: Change text color
    if data.startswith("\x1b[31m"):  # Red
        text_widget.tag_configure("red", foreground="red")
        text_widget.insert(tk.END, data[5:], "red")
    elif data.startswith("\x1b[32m"):  # Green
        text_widget.tag_configure("green", foreground="green")
        text_widget.insert(tk.END, data[5:], "green")
    elif data.startswith("\x1b[0m"):  # Reset to default text
        text_widget.tag_configure("reset", foreground="black")
        text_widget.insert(tk.END, data[4:], "reset")
    elif data.startswith("\x1b[1m"):  # Bold
        text_widget.tag_configure("bold", font=("Helvetica", 12, "bold"))
        text_widget.insert(tk.END, data[4:], "bold")
    elif data.startswith("\x1b[4m"):  # Underlined
        text_widget.tag_configure("underline", underline=True)
        text_widget.insert(tk.END, data[4:], "underline")
    else:
        text_widget.insert(tk.END, data)  # If no known escape sequence, simply insert

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

send_reset_phy_A_func_button = tk.Button(com_port_command, text="PHY Reset A", command=send_reset_phy_A_func)
send_reset_phy_A_func_button.pack(side=tk.LEFT)

send_reset_phy_B_func_button = tk.Button(com_port_command, text="PHY Reset B", command=send_reset_phy_B_func)
send_reset_phy_B_func_button.pack(side=tk.LEFT)

send_reset_phy_C_func_button = tk.Button(com_port_command, text="PHY Reset C", command=send_reset_phy_C_func)
send_reset_phy_C_func_button.pack(side=tk.LEFT)

send_reset_phy_D_func_button = tk.Button(com_port_command, text="PHY Reset D", command=send_reset_phy_D_func)
send_reset_phy_D_func_button.pack(side=tk.LEFT)

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
text_widget_A.pack(side=tk.LEFT, padx=10, pady=10, fill=tk.BOTH, expand=True)

# Second text widget for COM4 output
text_widget_B = scrolledtext.ScrolledText(top_text_widgets_frame, width=40, height=15)
text_widget_B.tag_configure("green_on_black", foreground="light green", background="black", font=("Helvetica", 12, "bold"))
text_widget_B.pack(side=tk.LEFT, padx=10, pady=10, fill=tk.BOTH, expand=True)

# Create a frame for the bottom two text widgets (COM5 and COM6)
bottom_text_widgets_frame = tk.Frame(root)
bottom_text_widgets_frame.pack(side=tk.TOP, fill=tk.BOTH, expand=True)

# Text widget for additional output (e.g., COM5)
text_widget_C = scrolledtext.ScrolledText(bottom_text_widgets_frame, width=40, height=15)
text_widget_C.tag_configure("green_on_black", foreground="light green", background="black", font=("Helvetica", 12, "bold"))
text_widget_C.pack(side=tk.LEFT, padx=10, pady=10, fill=tk.BOTH, expand=True)

# Text widget for additional output (e.g., COM6)
text_widget_D = scrolledtext.ScrolledText(bottom_text_widgets_frame, width=40, height=15)
text_widget_D.tag_configure("green_on_black", foreground="light green", background="black", font=("Helvetica", 12, "bold"))
text_widget_D.pack(side=tk.LEFT, padx=10, pady=10, fill=tk.BOTH, expand=True)
###################################################################################################

# Event binding for the text input field
text_widget_A.bind("<Return>", lambda event, text_widget=text_widget_A: send_to_com_port(serial_A, text_widget.get("insert linestart", "insert lineend")))
text_widget_B.bind("<Return>", lambda event, text_widget=text_widget_B: send_to_com_port(serial_B, text_widget.get("insert linestart", "insert lineend")))
text_widget_C.bind("<Return>", lambda event, text_widget=text_widget_C: send_to_com_port(serial_C, text_widget.get("insert linestart", "insert lineend")))
text_widget_D.bind("<Return>", lambda event, text_widget=text_widget_D: send_to_com_port(serial_D, text_widget.get("insert linestart", "insert lineend")))

# Start the Tkinter main loop
root.mainloop()
