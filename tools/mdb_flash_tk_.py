from distutils.sysconfig import project_base
import subprocess
import tkinter as tk
from tkinter import scrolledtext
import threading
import queue

import sys
import re
import os.path
import argparse
import time

mdb_A = None
mdb_B = None
mdb_C = None
mdb_D = None

hw_A = None
hw_B = None
hw_C = None
hw_D = None

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

HEX_FILE_01="..\\apps\\tcpip_iperf_lan867x\\firmware\\tcpip_iperf_lan867x_freertos.X\\dist\\FreeRTOS\\production\\tcpip_iperf_lan867x_freertos.X.production.hex"
HEX_FILE_02="..\\apps\\tcpip_iperf_lan867x\\firmware\\tcpip_iperf_lan867x_freertos.X\\dist\\FreeRTOS_node_1\\production\\tcpip_iperf_lan867x_freertos.X.production.hex"
HEX_FILE_03="..\\apps\\tcpip_iperf_lan867x\\firmware\\tcpip_iperf_lan867x_freertos.X\\dist\\FreeRTOS_node_2\\production\\tcpip_iperf_lan867x_freertos.X.production.hex"
HEX_FILE_04="..\\apps\\tcpip_iperf_lan867x\\firmware\\tcpip_iperf_lan867x_freertos.X\\dist\\FreeRTOS_node_3\\production\\tcpip_iperf_lan867x_freertos.X.production.hex"
MDB_PATH="c:\\Program Files\\Microchip\\MPLABX\\v6.15\\mplab_platform\\bin\\mdb.bat"
HW_TOOL="EDBG"
TG_MCU="ATSAME54P20A"
HW_SERIAL_01="ATML3264031800001044"




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

    print("Thread B stopped")



def run_mdb_All():
    global block_output
    block_output = False

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
    #input_queue_A.put("quit\n")    



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
    #input_queue_B.put("quit\n")    


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
    #input_queue_B.put("quit\n")    


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
    #input_queue_B.put("quit\n")    



def send_cmd_A(cmd):
    global input_queue_A
    global output_queue_A
    global output_text_A
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
    global HEX_FILE_01    
    send_cmd_A("program " + HEX_FILE_01 + "\n")

def run_prog_B():
    global HEX_FILE_02
    send_cmd_B('program ' + HEX_FILE_02 + '\n')

def run_prog_C():
    global HEX_FILE_03
    send_cmd_C('program ' + HEX_FILE_03 + '\n')

def run_prog_D():
    global HEX_FILE_03
    send_cmd_D('program ' + HEX_FILE_04 + '\n')




def stop_A_mdb():
    global mdb_A
    global mdb_B
    global mdb_C
    global mdb_D
    
    if mdb_A is not None:
        try:
            out = mdb_A.send('quit\n')
        except StopIteration:
            pass  # Ignore StopIteration when sending 'quit'

    if mdb_B is not None:
        try:
            out = mdb_B.send('quit\n')
        except StopIteration:
            pass  # Ignore StopIteration when sending 'quit'

    if mdb_C is not None:
        try:
            out = mdb_C.send('quit\n')
        except StopIteration:
            pass  # Ignore StopIteration when sending 'quit'

    if mdb_D is not None:
        try:
            out = mdb_D.send('quit\n')
        except StopIteration:
            pass  # Ignore StopIteration when sending 'quit'                






# Tkinter-GUI erstellen
root = tk.Tk()
root.title("MDB Output GUI")
root.geometry("1000x600") 

# Label und Eingabefelder für die Parameter
hex_file_label = tk.Label(root, text="Hex-Datei:")
hex_file_label.pack()
hex_file_entry = tk.Entry(root, width=160)
hex_file_entry.insert(0, HEX_FILE_01)  # Verwende den Standardwert
hex_file_entry.pack()

mdb_path_label = tk.Label(root, text="Pfad zu mdb:")
mdb_path_label.pack()
mdb_path_entry = tk.Entry(root, width=80)
mdb_path_entry.insert(0, MDB_PATH)  # Verwende den Standardwert
mdb_path_entry.pack()

hw_tool_label = tk.Label(root, text="Hardware-Tool:")
hw_tool_label.pack()
hw_tool_entry = tk.Entry(root, width=80)
hw_tool_entry.insert(0, HW_TOOL)  # Verwende den Standardwert
hw_tool_entry.pack()

tg_mcu_label = tk.Label(root, text="Ziel-MCU:")
tg_mcu_label.pack()
tg_mcu_entry = tk.Entry(root, width=80)
tg_mcu_entry.insert(0, TG_MCU)  # Verwende den Standardwert
tg_mcu_entry.pack()

hw_serial_label = tk.Label(root, text="Hardware-Tool-Seriennummer:")
hw_serial_label.pack()
hw_serial_entry = tk.Entry(root, width=80)
hw_serial_entry.insert(0, HW_SERIAL_01)  # Verwende den Standardwert
hw_serial_entry.pack()

# Button zum Starten des mdb-Prozesses und Anzeigen der Ausgabe

button_frame_1 = tk.Frame(root)
button_frame_1.pack()

hw_A = 0
hw_B = 1
hw_C = 2
hw_D = 3


start_button_All = tk.Button(button_frame_1, text="Start MDB All", command=run_mdb_All)
start_button_All.pack(side=tk.LEFT, padx=10) 

start_button_A = tk.Button(button_frame_1, text="Start MDB A", command=lambda: run_mdb_x(mdb_A, output_text_A,hw_A))
start_button_A.pack(side=tk.LEFT, padx=10) 

start_button_B = tk.Button(button_frame_1, text="Start MDB B", command=lambda: run_mdb_x(mdb_B, output_text_B,hw_B))
start_button_B.pack(side=tk.LEFT, padx=10)

start_button_C = tk.Button(button_frame_1, text="Start MDB C", command=lambda: run_mdb_x(mdb_C, output_text_C,hw_C))
start_button_C.pack(side=tk.LEFT, padx=10)

start_button_D = tk.Button(button_frame_1, text="Start MDB D", command=lambda: run_mdb_x(mdb_D, output_text_D,hw_D))
start_button_D.pack(side=tk.LEFT, padx=10)

prog_button_A = tk.Button(button_frame_1, text="Prog All", command=run_prog_All)
prog_button_A.pack(side=tk.LEFT, padx=10)

prog_button_B = tk.Button(button_frame_1, text="Prog B", command=run_prog_B)
prog_button_B.pack(side=tk.LEFT, padx=10)

prog_button_C = tk.Button(button_frame_1, text="Prog C", command=run_prog_C)
prog_button_C.pack(side=tk.LEFT, padx=10)

prog_button_D = tk.Button(button_frame_1, text="Prog D", command=run_prog_D)
prog_button_D.pack(side=tk.LEFT, padx=10)

stop_A_button_D = tk.Button(button_frame_1, text="Stop A", command=stop_A_mdb)
stop_A_button_D.pack(side=tk.LEFT, padx=10)



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


#hex_file = hex_file_entry.get()
mdb_path = mdb_path_entry.get()
#hw_tool = hw_tool_entry.get()
#tg_mcu = tg_mcu_entry.get()
#hw_serial = hw_serial_entry.get()


proc_A = subprocess.Popen(
    [mdb_path],
    stdin=subprocess.PIPE,
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE        
)
print("Process A start")
output_text_A.insert(tk.END, "MDB started...\n") 
output_text_A.see(tk.END)
root.update_idletasks()
output_queue_A = queue.Queue()
input_queue_A = queue.Queue()
thread_A = threading.Thread(target=mdb_communicator_thread_A)
thread_A.start()
while not thread_A.is_alive(): pass




proc_B = subprocess.Popen(
    [mdb_path],
    stdin=subprocess.PIPE,
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE        
)
print("Process B start")
output_text_B.insert(tk.END, "MDB started...\n") 
output_text_B.see(tk.END)
output_text_B.update_idletasks()
output_queue_B = queue.Queue()
input_queue_B = queue.Queue()
thread_B = threading.Thread(target=mdb_communicator_thread_B)
thread_B.start()
while not thread_B.is_alive(): pass



proc_C = subprocess.Popen(
    [mdb_path],
    stdin=subprocess.PIPE,
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE        
)
print("Process C start")
output_text_C.insert(tk.END, "MDB started...\n") 
output_text_C.see(tk.END)
output_text_C.update_idletasks()
output_queue_C = queue.Queue()
input_queue_C = queue.Queue()
thread_C = threading.Thread(target=mdb_communicator_thread_C)
thread_C.start()
while not thread_C.is_alive(): pass




proc_D = subprocess.Popen(
    [mdb_path],
    stdin=subprocess.PIPE,
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE        
)
print("Process D start")
output_text_D.insert(tk.END, "MDB started...\n") 
output_text_D.see(tk.END)
output_text_D.update_idletasks()
output_queue_D = queue.Queue()
input_queue_D = queue.Queue()
thread_D = threading.Thread(target=mdb_communicator_thread_D)
thread_D.start()
while not thread_D.is_alive(): pass



# wait for Prombt
out = output_queue_A.get()
output_text_A.insert(tk.END, out)
output_text_A.see(tk.END)
output_text_A.update_idletasks()
# wait for Prombt
out = output_queue_B.get()
output_text_B.insert(tk.END, out)
output_text_B.see(tk.END)
output_text_B.update_idletasks()
# wait for Prombt
out = output_queue_C.get()
output_text_C.insert(tk.END, out)
output_text_C.see(tk.END)
output_text_C.update_idletasks()
# wait for Prombt
out = output_queue_D.get()
output_text_D.insert(tk.END, out)
output_text_D.see(tk.END)
output_text_D.update_idletasks()




# Starte die GUI-Schleife
root.mainloop()

