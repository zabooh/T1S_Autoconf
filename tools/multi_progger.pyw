from distutils.sysconfig import project_base
import subprocess
import tkinter as tk
from tkinter import scrolledtext
from tkinter import filedialog
import threading
import queue
import re

import sys
import re
import os.path
import argparse
import time

hw_A = 1
hw_B = 3
hw_C = 2
hw_D = 0

tool_entry_A = None
tool_entry_B = None
tool_entry_C = None
tool_entry_D = None

proc_A = None
proc_B = None
proc_C = None
proc_D = None

output_queue_A = None
output_queue_B = None
output_queue_C = None
output_queue_D = None

input_queue_A = None
input_queue_B = None
input_queue_C = None
input_queue_D = None

block_output = True

output_text_A = None
output_text_B = None
output_text_C = None
output_text_D = None

index_numbers = None
serial_numbers = None

serial_label_A = None
serial_label_B = None
serial_label_C =  None

serial_label_D = None 

hex_file_entry_A = None
hex_file_entry_B = None
hex_file_entry_C = None
hex_file_entry_D = None

HEX_FILE_01="..\\apps\\tcpip_iperf_lan867x\\firmware\\tcpip_iperf_lan867x_freertos.X\\dist\\FreeRTOS\\production\\tcpip_iperf_lan867x_freertos.X.production.hex"
HEX_FILE_02="..\\apps\\tcpip_iperf_lan867x\\firmware\\tcpip_iperf_lan867x_freertos.X\\dist\\FreeRTOS\\production\\tcpip_iperf_lan867x_freertos.X.production.hex"
HEX_FILE_03="..\\apps\\tcpip_iperf_lan867x\\firmware\\tcpip_iperf_lan867x_freertos.X\\dist\\FreeRTOS\\production\\tcpip_iperf_lan867x_freertos.X.production.hex"
HEX_FILE_04="..\\apps\\tcpip_iperf_lan867x\\firmware\\tcpip_iperf_lan867x_freertos.X\\dist\\FreeRTOS\\production\\tcpip_iperf_lan867x_freertos.X.production.hex"
MDB_PATH="c:\\Program Files\\Microchip\\MPLABX\\v6.15\\mplab_platform\\bin\\mdb.bat"
HW_TOOL="EDBG"
TG_MCU="ATSAME54P20A"
HW_SERIAL_01="ATML3264031800001044"

gui_thread = None
start_gui_flag = False
selected_file_label = None
hwtool_out = None

current_directory = None
stop_immediately = False


def mdb_communicator_thread_A():
    global proc_A
    global output_queue_A
    global input_queue_A
    global block_output
    flag_read = False
    flag_write = True

    string = ''

    while True:
        if proc_A.poll() is not None:
            break

        if flag_write == True:
            byte = ''
            available_byte = True
            try:
                if available_byte:
                    byte = proc_A.stdout.read(1)

                    if block_output == False:
                        output_text_A.insert(tk.END, byte)
                        output_text_A.see(tk.END)
                        output_text_A.update_idletasks()

                    string += byte.decode('UTF-8')
                    if string[-1] == '>':
                        byte = ''
                        output_queue_A.put(string)
                        flag_read = True
                        flag_write = False
                        string = ''
            except:
                pass

        if flag_read == True:            
            try:
                if not input_queue_A.empty():
                    flag_read = False
                    flag_write = True
                    to_send = input_queue_A.get_nowait()
                    proc_A.stdin.write(to_send.encode('utf-8'))
                    proc_A.stdin.flush()
            except:
                pass

    print("Thread A stopped")
    if block_output == False:
        output_text_A.insert(tk.END, "Thread A stopped")
        output_text_A.see(tk.END)
        output_text_A.update_idletasks()
    proc_A.terminate()
    proc_B.terminate()
    proc_C.terminate()
    proc_D.terminate()  

def mdb_communicator_thread_B():
    global proc_B
    global output_queue_B
    global input_queue_B
    flag_read = False
    flag_write = True

    string = ''

    while True:
        if proc_B.poll() is not None:
            break

        if flag_write == True:
            byte = ''
            available_byte = True
            try:
                if available_byte:
                    byte = proc_B.stdout.read(1)

                    if block_output == False:
                        output_text_B.insert(tk.END, byte)
                        output_text_B.see(tk.END)
                        output_text_B.update_idletasks()

                    string += byte.decode('UTF-8')
                    if string[-1] == '>':
                        byte = ''
                        output_queue_B.put(string)
                        flag_read = True
                        flag_write = False
                        string = ''
            except:
                pass

        if flag_read == True:            
            try:
                if not input_queue_B.empty():
                    flag_read = False
                    flag_write = True
                    to_send = input_queue_B.get_nowait()
                    proc_B.stdin.write(to_send.encode('utf-8'))
                    proc_B.stdin.flush()
            except:
                pass

    print("Thread B stopped")
    if block_output == False:
        output_text_B.insert(tk.END, "Thread B stopped")
        output_text_B.see(tk.END)
        output_text_B.update_idletasks()
    proc_A.terminate()
    proc_B.terminate()
    proc_C.terminate()
    proc_D.terminate()  

def mdb_communicator_thread_C():
    global proc_C
    global output_queue_C
    global input_queue_C
    flag_read = False
    flag_write = True

    string = ''

    while True:
        if proc_C.poll() is not None:
            break

        if flag_write == True:
            byte = ''
            available_byte = True
            try:
                if available_byte:
                    byte = proc_C.stdout.read(1)

                    if block_output == False:
                        output_text_C.insert(tk.END, byte)
                        output_text_C.see(tk.END)
                        output_text_C.update_idletasks()

                    string += byte.decode('UTF-8')
                    if string[-1] == '>':
                        byte = ''
                        output_queue_C.put(string)
                        flag_read = True
                        flag_write = False
                        string = ''
            except:
                pass

        if flag_read == True:            
            try:
                if not input_queue_C.empty():
                    flag_read = False
                    flag_write = True
                    to_send = input_queue_C.get_nowait()
                    proc_C.stdin.write(to_send.encode('utf-8'))
                    proc_C.stdin.flush()
            except:
                pass

    print("Thread C stopped")
    if block_output == False:
        output_text_C.insert(tk.END, "Thread C stopped")
        output_text_C.see(tk.END)
        output_text_C.update_idletasks()
    proc_A.terminate()
    proc_B.terminate()
    proc_C.terminate()
    proc_D.terminate()  

def mdb_communicator_thread_D():
    global proc_D
    global output_queue_D
    global input_queue_D
    flag_read = False
    flag_write = True

    string = ''

    while True:
        if proc_D.poll() is not None:
            break

        if flag_write == True:
            byte = ''
            available_byte = True
            try:
                if available_byte:
                    byte = proc_D.stdout.read(1)

                    if block_output == False:
                        output_text_D.insert(tk.END, byte)
                        output_text_D.see(tk.END)
                        output_text_D.update_idletasks()

                    string += byte.decode('UTF-8')
                    if string[-1] == '>':
                        byte = ''
                        output_queue_D.put(string)
                        flag_read = True
                        flag_write = False
                        string = ''
            except:
                pass

        if flag_read == True:            
            try:
                if not input_queue_D.empty():
                    flag_read = False
                    flag_write = True
                    to_send = input_queue_D.get_nowait()
                    proc_D.stdin.write(to_send.encode('utf-8'))
                    proc_D.stdin.flush()
            except:
                pass

    print("Thread D stopped")
    if block_output == False:
        output_text_D.insert(tk.END, "Thread D stopped")
        output_text_D.see(tk.END)
        output_text_D.update_idletasks()
    proc_A.terminate()
    proc_B.terminate()
    proc_C.terminate()
    proc_D.terminate()  

    


def Stop_All():
    stop_mdb_All()
    sys.exit()


def run_mdb_All():
    global block_output
    block_output = False
    global hw_A
    global hw_B
    global hw_C
    global hw_D
    global tool_entry_A
    global tool_entry_B
    global tool_entry_C
    global tool_entry_D

    hw_A = int(tool_entry_A.get())
    hw_B = int(tool_entry_B.get())
    hw_C = int(tool_entry_C.get())
    hw_D = int(tool_entry_D.get())

    thread_run_A = threading.Thread(target=run_mdb_A)
    thread_run_A.start()
    while not thread_run_A.is_alive(): pass
    
    thread_run_B = threading.Thread(target=run_mdb_B)
    thread_run_B.start()
    while not thread_run_B.is_alive(): pass

    thread_run_C = threading.Thread(target=run_mdb_C)
    thread_run_C.start()
    while not thread_run_C.is_alive(): pass

    thread_run_D = threading.Thread(target=run_mdb_D)
    thread_run_D.start()
    while not thread_run_D.is_alive(): pass    

def run_mdb_A():
    global proc_A
    global input_queue_A
    global output_queue_A
    global HW_TOOL
    global TG_MCU
    global mdb_path

    send_cmd_A("device "+ TG_MCU + "\n")
    send_cmd_A("set AutoSelectMemRanges auto\n")
    send_cmd_A("set communication.interface swd\n")
    send_cmd_A("set communication.speed 6.000\n")
    send_cmd_A("hwtool " + HW_TOOL + " -p " + str(hw_A) + "\n")


def run_mdb_B():
    global proc_B
    global input_queue_B
    global output_queue_B
    global HW_TOOL
    global TG_MCU
    global mdb_path

    send_cmd_B("device "+ TG_MCU + "\n")
    send_cmd_B("set AutoSelectMemRanges auto\n")
    send_cmd_B("set communication.interface swd\n")
    send_cmd_B("set communication.speed 6.000\n")
    send_cmd_B("hwtool " + HW_TOOL + " -p " + str(hw_B) + "\n")

def run_mdb_C():
    global proc_C
    global input_queue_C
    global output_queue_C
    global HW_TOOL
    global TG_MCU
    global mdb_path

    send_cmd_C("device "+ TG_MCU + "\n")
    send_cmd_C("set AutoSelectMemRanges auto\n")
    send_cmd_C("set communication.interface swd\n")
    send_cmd_C("set communication.speed 6.000\n")
    send_cmd_C("hwtool " + HW_TOOL + " -p " + str(hw_C) + "\n")

def run_mdb_D():
    global proc_D
    global input_queue_D
    global output_queue_D
    global HW_TOOL
    global TG_MCU
    global mdb_path

    send_cmd_D("device "+ TG_MCU + "\n")
    send_cmd_D("set AutoSelectMemRanges auto\n")
    send_cmd_D("set communication.interface swd\n")
    send_cmd_D("set communication.speed 6.000\n")
    send_cmd_D("hwtool " + HW_TOOL + " -p " + str(hw_D) + "\n")

def run_hwtool_A():
    global block_output
    block_output = False

    thread_hwtool_A = threading.Thread(target=thread_run_hwtool_A)
    thread_hwtool_A.start()
    while not thread_hwtool_A.is_alive(): pass



def thread_run_hwtool_A():
    global hwtool_out
    global index_numbers
    global serial_numbers
    global serial_label_A
    global serial_label_B
    global serial_label_C
    global serial_label_D
    global hw_A
    global hw_B
    global hw_C
    global hw_D    
    global tool_entry_A
    global tool_entry_B
    global tool_entry_C
    global tool_entry_D

    hwtool_out = send_cmd_A("hwtool\n")
    print(":"+hwtool_out+":")
    index_numbers, serial_numbers = extract_serial_and_index(hwtool_out)

    print("Index Numbers:", index_numbers)
    print("Serial Numbers:", serial_numbers)

    hw_A = int(tool_entry_A.get())
    hw_B = int(tool_entry_B.get())
    hw_C = int(tool_entry_C.get())
    hw_D = int(tool_entry_D.get())

    serial_label_A.config(text=f"{serial_numbers[hw_A]}")    
    serial_label_B.config(text=f"{serial_numbers[hw_B]}")    
    serial_label_C.config(text=f"{serial_numbers[hw_C]}")    
    serial_label_D.config(text=f"{serial_numbers[hw_D]}")    

def send_cmd_A(cmd):
    global input_queue_A
    global output_queue_A
    global output_text_A
    while not output_queue_A.empty(): output_queue_A.get()
    input_queue_A.put(cmd)
    output_text_A.insert(tk.END, cmd)
    output_text_A.see(tk.END)
    output_text_A.update_idletasks()
    out = output_queue_A.get()
    return out
    
def send_cmd_B(cmd):
    global input_queue_B
    global output_queue_B
    global output_text_B
    while not output_queue_B.empty(): output_queue_B.get()
    input_queue_B.put(cmd)
    output_text_B.insert(tk.END, cmd)
    output_text_B.see(tk.END)
    output_text_B.update_idletasks()
    out = output_queue_B.get();    
    return out
    
def send_cmd_C(cmd):
    global input_queue_C
    global output_queue_C
    global output_text_C
    while not output_queue_C.empty(): output_queue_C.get()
    input_queue_C.put(cmd)
    output_text_C.insert(tk.END, cmd)
    output_text_C.see(tk.END)
    output_text_C.update_idletasks()
    out = output_queue_C.get();    
    return out
    
def send_cmd_D(cmd):
    global input_queue_D
    global output_queue_D
    global output_text_D
    while not output_queue_D.empty(): output_queue_D.get()
    input_queue_D.put(cmd)
    output_text_D.insert(tk.END, cmd)
    output_text_D.see(tk.END)
    output_text_D.update_idletasks()
    out = output_queue_D.get();    
    return out
    



def run_prog_All():
    thread_A = threading.Thread(target=run_prog_A)
    thread_A.start()    
    thread_B = threading.Thread(target=run_prog_B)
    thread_B.start()
    thread_C = threading.Thread(target=run_prog_C)
    thread_C.start()
    thread_D = threading.Thread(target=run_prog_D)
    thread_D.start()

def run_prog_A():
    global current_directory
    global hex_file_entry_A
    send_cmd_A("program " + "\"" + current_directory + "\\" + hex_file_entry_A.get() + "\""  + "\n")
    send_cmd_A("reset" + "\n")
    send_cmd_A("run" + "\n")

def run_prog_B():
    global current_directory
    global hex_file_entry_B
    send_cmd_B('program ' + "\"" +  current_directory  + "\\" + hex_file_entry_B.get() + "\"" + '\n')
    send_cmd_B("reset" + "\n")
    send_cmd_B("run" + "\n")

def run_prog_C():
    global current_directory
    global hex_file_entry_C
    send_cmd_C('program ' + "\"" + current_directory  + "\\" + hex_file_entry_C.get()+ "\""  + '\n')
    send_cmd_C("reset" + "\n")
    send_cmd_C("run" + "\n")
    
def run_prog_D():
    global current_directory
    global hex_file_entry_D
    send_cmd_D('program ' + "\"" + current_directory  + "\\" + hex_file_entry_D.get() + "\"" + '\n')
    send_cmd_D("reset" + "\n")
    send_cmd_D("run" + "\n")

def stop_mdb_All():
    global gui_thread
    global block_output
    global stop_immediately
    if stop_immediately == False:
        print("All stopped....")
        block_output = True
        input_queue_A.put("quit\n")
        input_queue_B.put("quit\n")
        input_queue_C.put("quit\n")
        input_queue_D.put("quit\n")
        proc_A.terminate()
        proc_B.terminate()
        proc_C.terminate()
        proc_D.terminate()    
        time.sleep(0.5)
    root.quit()
    root.destroy()
    sys.exit()
    


def exit_program():
    root.quit()  # Beendet die Tkinter-Hauptschleife (main loop)


def open_file_dialog():
    global selected_file_label

    file_path = filedialog.askopenfilename()
    if file_path:
        selected_file_label.config(text="Ausgewählte Datei: " + file_path)
    else:
        selected_file_label.config(text="Keine Datei ausgewählt")


def start_gui():
    global root
    global mdb_path_entry
    global start_gui_flag
    global output_text_A
    global output_text_B
    global output_text_C
    global output_text_D
    global tool_entry_A
    global tool_entry_B
    global tool_entry_C
    global tool_entry_D    
    global serial_label_A
    global serial_label_B
    global serial_label_C
    global serial_label_D
    global selected_file_label
    global hex_file_entry_A
    global hex_file_entry_B
    global hex_file_entry_C
    global hex_file_entry_D

    # Tkinter-GUI erstellen
    root = tk.Tk()
    root.title("MDB Output GUI")
    root.geometry("1400x600") 

    # Container-Frame für die Labels und Eingabefelder erstellen
    hex_file_frame_A = tk.Frame(root)
    hex_file_frame_A.pack(pady=1, padx=1)
    # Label für Hex-Datei A erstellen und im Container platzieren
    hex_file_label_A = tk.Label(hex_file_frame_A, text="Hex-Datei A:")
    hex_file_label_A.pack(side=tk.LEFT, padx=5)
    # Eingabefeld für Hex-Datei A erstellen und im Container platzieren
    hex_file_entry_A = tk.Entry(hex_file_frame_A, width=150)
    hex_file_entry_A.insert(0, HEX_FILE_01)  # Verwende den Standardwert
    hex_file_entry_A.pack(side=tk.LEFT, padx=5)
    # Tool Index
    tool_label_A = tk.Label(hex_file_frame_A, text="Tool Index")
    tool_label_A.pack(side=tk.LEFT, padx=5)    
    tool_entry_A = tk.Entry(hex_file_frame_A, width=5)
    tool_entry_A.insert(0, hw_A)  # Verwende den Standardwert
    tool_entry_A.pack(side=tk.LEFT, padx=5)
    # Serial Number
    serial_label_A = tk.Label(hex_file_frame_A, text=f"Serial: {'?'}")
    serial_label_A.pack(side=tk.LEFT)


    # Container-Frame für die Labels und Eingabefelder erstellen
    hex_file_frame_B = tk.Frame(root)
    hex_file_frame_B.pack(pady=1, padx=1)
    # Label für Hex-Datei A erstellen und im Container platzieren
    hex_file_label_B = tk.Label(hex_file_frame_B, text="Hex-Datei B:")
    hex_file_label_B.pack(side=tk.LEFT, padx=5)
    # Eingabefeld für Hex-Datei A erstellen und im Container platzieren
    hex_file_entry_B = tk.Entry(hex_file_frame_B, width=150)
    hex_file_entry_B.insert(0, HEX_FILE_02)  # Verwende den Standardwert
    hex_file_entry_B.pack(side=tk.LEFT, padx=5)
    # Tool Index
    tool_label_B = tk.Label(hex_file_frame_B, text="Tool Index")
    tool_label_B.pack(side=tk.LEFT, padx=5)    
    tool_entry_B = tk.Entry(hex_file_frame_B, width=5)
    tool_entry_B.insert(0, hw_B)  # Verwende den Standardwert
    tool_entry_B.pack(side=tk.LEFT, padx=5)
    # Serial Number
    serial_label_B = tk.Label(hex_file_frame_B, text=f"Serial: {'?'}")
    serial_label_B.pack(side=tk.LEFT)


    # Container-Frame für die Labels und Eingabefelder erstellen
    hex_file_frame_C = tk.Frame(root)
    hex_file_frame_C.pack(pady=1, padx=1)
    # Label für Hex-Datei A erstellen und im Container platzieren
    hex_file_label_C = tk.Label(hex_file_frame_C, text="Hex-Datei C:")
    hex_file_label_C.pack(side=tk.LEFT, padx=5)
    # Eingabefeld für Hex-Datei A erstellen und im Container platzieren
    hex_file_entry_C = tk.Entry(hex_file_frame_C, width=150)
    hex_file_entry_C.insert(0, HEX_FILE_03)  # Verwende den Standardwert
    hex_file_entry_C.pack(side=tk.LEFT, padx=5)
    # Tool Index
    tool_label_C = tk.Label(hex_file_frame_C, text="Tool Index")
    tool_label_C.pack(side=tk.LEFT, padx=5)    
    tool_entry_C = tk.Entry(hex_file_frame_C, width=5)
    tool_entry_C.insert(0, hw_C)  # Verwende den Standardwert
    tool_entry_C.pack(side=tk.LEFT, padx=5)
    # Serial Number
    serial_label_C = tk.Label(hex_file_frame_C, text=f"Serial: {'?'}")
    serial_label_C.pack(side=tk.LEFT)


    # Container-Frame für die Labels und Eingabefelder erstellen
    hex_file_frame_D = tk.Frame(root)
    hex_file_frame_D.pack(pady=1, padx=1)
    # Label für Hex-Datei A erstellen und im Container platzieren
    hex_file_label_D = tk.Label(hex_file_frame_D, text="Hex-Datei D:")
    hex_file_label_D.pack(side=tk.LEFT, padx=5)
    # Eingabefeld für Hex-Datei A erstellen und im Container platzieren
    hex_file_entry_D = tk.Entry(hex_file_frame_D, width=150)
    hex_file_entry_D.insert(0, HEX_FILE_03)  # Verwende den Standardwert
    hex_file_entry_D.pack(side=tk.LEFT, padx=5)
    # Tool Index
    tool_label_D = tk.Label(hex_file_frame_D, text="Tool Index")
    tool_label_D.pack(side=tk.LEFT, padx=5)    
    tool_entry_D = tk.Entry(hex_file_frame_D, width=5)
    tool_entry_D.insert(0, hw_D)  # Verwende den Standardwert
    tool_entry_D.pack(side=tk.LEFT, padx=5)
    # Serial Number
    serial_label_D = tk.Label(hex_file_frame_D, text=f"Serial: {'?'}")
    serial_label_D.pack(side=tk.LEFT)

    # Container-Frame für die Labels und Eingabefelder erstellen
    mdb_path_frame_D = tk.Frame(root)
    mdb_path_frame_D.pack(pady=1, padx=1)
    mdb_path_label = tk.Label(mdb_path_frame_D, text="Pfad zu mdb:")
    mdb_path_label.pack(side=tk.LEFT, padx=5)
    mdb_path_entry = tk.Entry(mdb_path_frame_D, width=80)
    mdb_path_entry.insert(0, MDB_PATH)  # Verwende den Standardwert
    mdb_path_entry.pack(side=tk.LEFT, padx=5)


    button_frame_1 = tk.Frame(root)
    button_frame_1.pack()

    start_button_All = tk.Button(button_frame_1, text="Connect MDB", command=run_mdb_All)
    start_button_All.pack(side=tk.LEFT, padx=1) 

    prog_button_A = tk.Button(button_frame_1, text="Prog Boards", command=run_prog_All)
    prog_button_A.pack(side=tk.LEFT, padx=1)

    prog_button_A = tk.Button(button_frame_1, text="hwtool A", command=run_hwtool_A)
    prog_button_A.pack(side=tk.LEFT, padx=1)

    top_text_widgets_frame = tk.Frame(root)
    top_text_widgets_frame.pack(side=tk.TOP, fill=tk.BOTH, expand=True)

    output_text_A = scrolledtext.ScrolledText(top_text_widgets_frame, width=40, height=15)  #.ScrolledText(root, width=100, height=20, wrap=tk.WORD)
    output_text_A.tag_configure("green_on_black", foreground="light green", background="black", font=("Helvetica", 12, "bold"))
    output_text_A.pack(side=tk.LEFT, padx=10, pady=10, fill=tk.BOTH, expand=True)

    output_text_B = scrolledtext.ScrolledText(top_text_widgets_frame, width=40, height=15)  #.ScrolledText(root, width=100, height=20, wrap=tk.WORD)
    output_text_B.tag_configure("green_on_black", foreground="light green", background="black", font=("Helvetica", 12, "bold"))
    output_text_B.pack(side=tk.LEFT, padx=10, pady=10, fill=tk.BOTH, expand=True)

    bottom_text_widgets_frame = tk.Frame(root)
    bottom_text_widgets_frame.pack(side=tk.TOP, fill=tk.BOTH, expand=True)

    output_text_C = scrolledtext.ScrolledText(bottom_text_widgets_frame, width=40, height=15)  #.ScrolledText(root, width=100, height=20, wrap=tk.WORD)
    output_text_C.tag_configure("green_on_black", foreground="light green", background="black", font=("Helvetica", 12, "bold"))
    output_text_C.pack(side=tk.LEFT, padx=10, pady=10, fill=tk.BOTH, expand=True)

    output_text_D = scrolledtext.ScrolledText(bottom_text_widgets_frame, width=40, height=15)  #.ScrolledText(root, width=100, height=20, wrap=tk.WORD)
    output_text_D.tag_configure("green_on_black", foreground="light green", background="black", font=("Helvetica", 12, "bold"))
    output_text_D.pack(side=tk.LEFT, padx=10, pady=10, fill=tk.BOTH, expand=True)

    root.protocol("WM_DELETE_WINDOW", stop_mdb_All)
    
    # Erstellen Sie einen Button, der den Datei-Browser öffnet
    #open_file_button = tk.Button(hex_file_frame_A, text="File", command=open_file_dialog)
    #open_file_button.pack(side=tk.LEFT, padx=5)

    # Erstellen Sie ein Label, um den ausgewählten Dateipfad anzuzeigen
    #selected_file_label = tk.Label(root, text="Keine Datei ausgewählt")
    #selected_file_label.pack()

    start_gui_flag = True 
    root.mainloop()



def extract_serial_and_index(text):
    # Verwenden Sie einen regulären Ausdruck, um die Zeilen zu analysieren und die Seriennummern und Indexnummern zu extrahieren
    pattern = r'\s*(\d+)\s+\w+\s+(\w+)\s+.*'

    # Finden Sie alle Übereinstimmungen im Text
    matches = re.findall(pattern, text)

    # Initialisieren Sie leere Arrays für Seriennummern und Indexnummern
    index_numbers = []
    serial_numbers = []

    # Iterieren Sie über die Übereinstimmungen
    for match in matches:
        index, serial_number = match
        index_numbers.append(index)
        serial_numbers.append(serial_number)

    return index_numbers, serial_numbers




if __name__ == "__main__":

    gui_thread = threading.Thread(target=start_gui)
    gui_thread.start()
    while not gui_thread.is_alive(): pass
    while start_gui_flag == False: pass
    block_output = False

    #hex_file = hex_file_entry.get()
    mdb_path = mdb_path_entry.get()
    #hw_tool = hw_tool_entry.get()
    #tg_mcu = tg_mcu_entry.get()
    #hw_serial = hw_serial_entry.get()

    if os.path.exists(mdb_path):
        # Der Pfad existiert, starte den Prozess
        current_directory = os.getcwd()

        # Definieren Sie die Flags, um das Konsolenfenster zu verhindern
        CREATE_NO_WINDOW = 0x08000000

        proc_A = subprocess.Popen(
            [mdb_path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            creationflags=CREATE_NO_WINDOW,
            cwd=current_directory
        )
        print("Process A start")
        output_text_A.insert(tk.END, "MDB started...\n") 
        output_text_A.see(tk.END)
        root.update_idletasks()
        output_queue_A = queue.Queue()
        input_queue_A = queue.Queue()
        thread_A = threading.Thread(target=mdb_communicator_thread_A)
        time.sleep(0.5)
        thread_A.start()
        while not thread_A.is_alive(): pass    

        proc_B = subprocess.Popen(
            [mdb_path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            creationflags=CREATE_NO_WINDOW,
            cwd=current_directory
        )
        print("Process B start")
        output_text_B.insert(tk.END, "MDB started...\n") 
        output_text_B.see(tk.END)
        output_text_B.update_idletasks()
        output_queue_B = queue.Queue()
        input_queue_B = queue.Queue()
        thread_B = threading.Thread(target=mdb_communicator_thread_B)
        time.sleep(0.5)
        thread_B.start()
        while not thread_B.is_alive(): pass

        proc_C = subprocess.Popen(
            [mdb_path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            creationflags=CREATE_NO_WINDOW,
            cwd=current_directory
        )
        print("Process C start")
        output_text_C.insert(tk.END, "MDB started...\n") 
        output_text_C.see(tk.END)
        output_text_C.update_idletasks()
        output_queue_C = queue.Queue()
        input_queue_C = queue.Queue()
        thread_C = threading.Thread(target=mdb_communicator_thread_C)
        time.sleep(0.5)
        thread_C.start()
        while not thread_C.is_alive(): pass

        proc_D = subprocess.Popen(
            [mdb_path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            creationflags=CREATE_NO_WINDOW,
            cwd=current_directory
        )
        print("Process D start")
        output_text_D.insert(tk.END, "MDB started...\n") 
        output_text_D.see(tk.END)
        output_text_D.update_idletasks()
        output_queue_D = queue.Queue()
        input_queue_D = queue.Queue()
        thread_D = threading.Thread(target=mdb_communicator_thread_D)
        time.sleep(0.5)
        thread_D.start()
        while not thread_D.is_alive(): pass

        directory = os.path.dirname(os.path.abspath(__file__))
        os.chdir(directory)                                
        current_directory = os.getcwd()

    else:
        # Der Pfad existiert nicht, zeige eine Benachrichtigung an
        output_text_A.insert(tk.END, "The MDB Path does not exist. Please close the Program and change in the Source Code the following Line:\n")
        output_text_A.insert(tk.END, "\"MDB_PATH=\"c:\\Program Files\\Microchip\\MPLABX\\v6.10\\mplab_platform\\bin\\mdb.bat\"");
        stop_immediately = True

