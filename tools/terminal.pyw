import serial
import serial.tools.list_ports
import threading
import subprocess
import tkinter as tk
from tkinter import scrolledtext
import os  # Importieren Sie das 'os'-Modul für das Ausführen des anderen Python-Programms



#######################################################################################
# 
#   Init Variables
#
#######################################################################################


# Standard-COM-Port-Einstellungen
default_com_port_A = 'COM3'
default_com_port_B = 'COM4'
default_com_port_C = 'COM14'
default_com_port_D = 'COM21'
baud_rate = 115200


serA = None  # Serial-Objekt für COM-Port 1
serB = None  # Serial-Objekt für COM-Port 2
serC = None  # Serial-Objekt für COM-Port 3
serD = None  # Serial-Objekt für COM-Port 4
    

# Festlegen des Arbeitsverzeichnisses auf das Verzeichnis, in dem sich das Hauptprogramm befindet
working_directory = os.path.dirname(os.path.abspath(__file__))
os.chdir(working_directory)


#######################################################################################
# 
#   Functions
#
#######################################################################################

    
    
# Funktion zum Senden von Benutzereingaben an den COM-Port mit CR+LF
def send_to_com_port(ser, user_input):
    if user_input.strip().startswith(">"):
        user_input = user_input.strip()[1:]  # Entferne das ">" am Anfang
    user_input = user_input + "\r\n"  # CR+LF hinzufügen
    ser.write(user_input.encode())
    print(f"Gesendet an COM-Port: {user_input}")  # Debug-Ausgabe in der Konsole


# Funktion zum Herstellen der COM-Port-Verbindung
def connect_to_com_ports():
    global serA, serB, serC, serD, com_ports

    # Erneut alle verfügbaren COM-Ports abrufen
    com_ports = list(serial.tools.list_ports.comports())
    
    for port_info in com_ports:
        if port_info.serial_number:
            com_port_serial_dict[port_info.device] = port_info.serial_number
    
    update_com_port_label_x()
    
    com_port_A = com_port_entry_A.get()
    com_port_B = com_port_entry_B.get()    
    com_port_C = com_port_entry_C.get()
    com_port_D = com_port_entry_D.get()    
    
    print(f"Verbindung zu COM-Port's wird hergestellt...")

    try:
        serA = serial.Serial(com_port_A, baud_rate, timeout=0)
        serB = serial.Serial(com_port_B, baud_rate, timeout=0)
        serC = serial.Serial(com_port_C, baud_rate, timeout=0)
        serD = serial.Serial(com_port_D, baud_rate, timeout=0)

        print(f"Verbindung zu COM-Port {com_port_A}, {com_port_B}, {com_port_C} und {com_port_D} hergestellt.")

        # Threads zum Lesen von Daten von den COM-Ports erstellen und starten
        com_reader_thread_A = threading.Thread(target=read_from_com_port, args=(serA, text_widget_A))
        com_reader_thread_A.daemon = True
        com_reader_thread_A.start()

        com_reader_thread_B = threading.Thread(target=read_from_com_port, args=(serB, text_widget_B))
        com_reader_thread_B.daemon = True
        com_reader_thread_B.start()

        com_reader_thread_C = threading.Thread(target=read_from_com_port, args=(serC, text_widget_C))
        com_reader_thread_C.daemon = True
        com_reader_thread_C.start()

        com_reader_thread_D = threading.Thread(target=read_from_com_port, args=(serD, text_widget_D))
        com_reader_thread_D.daemon = True
        com_reader_thread_D.start()

        # Deaktiviere die COM-Port-Eingabefelder und den Connect-Button nach der Verbindung
        com_port_entry_A.config(state=tk.DISABLED)
        com_port_entry_B.config(state=tk.DISABLED)
        com_port_entry_C.config(state=tk.DISABLED)
        com_port_entry_D.config(state=tk.DISABLED)
        connect_button.config(state=tk.DISABLED)
        disconnect_button.config(state=tk.NORMAL)

    except serial.SerialException as e:
        print(f"Fehler bei der seriellen Verbindung: {e}")



# Funktion zum Trennen der COM-Port-Verbindung
def disconnect_from_com_ports():
    global serA, serB
    if serA is not None:
        serA.close()
    if serB is not None:
        serB.close()
    if serC is not None:
        serC.close()    
    if serD is not None:
        serD.close()

    # Aktiviere die COM-Port-Eingabefelder und den Connect-Button nach der Trennung
    com_port_entry_A.config(state=tk.NORMAL)
    com_port_entry_B.config(state=tk.NORMAL)
    com_port_entry_C.config(state=tk.NORMAL)
    com_port_entry_D.config(state=tk.NORMAL)
    connect_button.config(state=tk.NORMAL)
    disconnect_button.config(state=tk.DISABLED)


# Funktion zum Lesen von Daten vom COM-Port und Aktualisieren des GUI-Textfelds
def read_from_com_port(ser, text_widget):
    while True:
        try:
            data = ser.read(1)  # Ein Zeichen vom COM-Port lesen
            if data:
                #process_vt100_escape(text_widget, data.decode())  # Daten im Textfeld anzeigen und VT100 Escape-Sequenzen verarbeiten
                text_widget.insert(tk.END, data,"green_on_black")
                text_widget.see(tk.END)  # Zum Ende des Textfelds scrollen
        except serial.SerialException as e:
            print(f"Fehler bei der seriellen Verbindung: {e}")
            break


# Funktion zur Verarbeitung von VT100 Escape-Sequenzen
def process_vt100_escape(text_widget, data):
    # VT100 Escape-Sequenzen können hier verarbeitet werden
    # Beispiel: Ändern der Textfarbe
    if data.startswith("\x1b[31m"):  # Rot
        text_widget.tag_configure("red", foreground="red")
        text_widget.insert(tk.END, data[5:], "red")
    elif data.startswith("\x1b[32m"):  # Grün
        text_widget.tag_configure("green", foreground="green")
        text_widget.insert(tk.END, data[5:], "green")
    elif data.startswith("\x1b[0m"):  # Zurücksetzen auf Standardtext
        text_widget.tag_configure("reset", foreground="black")
        text_widget.insert(tk.END, data[4:], "reset")
    elif data.startswith("\x1b[1m"):  # Fett
        text_widget.tag_configure("bold", font=("Helvetica", 12, "bold"))
        text_widget.insert(tk.END, data[4:], "bold")
    elif data.startswith("\x1b[4m"):  # Unterstrichen
        text_widget.tag_configure("underline", underline=True)
        text_widget.insert(tk.END, data[4:], "underline")
    else:
        text_widget.insert(tk.END, data)  # Keine bekannte Escape-Sequenz, einfach einfügen


def start_terminal_program():
    # Hier den Pfad zum Python-Programm "terminal.pyw" angeben
    terminal_program_path = ".\check_serial_tk.pyw"

    # Über das 'subprocess' Modul das andere Python-Programm ausführen
    subprocess.Popen(["pythonw", terminal_program_path])


def update_com_port_label_x():
   com_port_label_A.config(text=f"COM-Port A: {com_port_serial_dict.get(com_port_entry_A.get(), 'Nicht verfügbar')}")    
   com_port_label_B.config(text=f"COM-Port B: {com_port_serial_dict.get(com_port_entry_B.get(), 'Nicht verfügbar')}")
   com_port_label_C.config(text=f"COM-Port C: {com_port_serial_dict.get(com_port_entry_C.get(), 'Nicht verfügbar')}")    
   com_port_label_D.config(text=f"COM-Port D: {com_port_serial_dict.get(com_port_entry_D.get(), 'Nicht verfügbar')}")

def clear_text():
    text_widget_A.delete(1.0, tk.END)
    text_widget_B.delete(1.0, tk.END)    
    text_widget_C.delete(1.0, tk.END)    
    text_widget_D.delete(1.0, tk.END)    
        
def send_left_command_func():
    send_to_com_port(serA,"iperf -u -s")

def send_right_command_func():
    send_to_com_port(serB,"iperf -u -c 192.168.100.11")

def send_netinfo_func():
    send_to_com_port(serA,"netinfo")
    send_to_com_port(serB,"netinfo")
    send_to_com_port(serC,"netinfo")
    send_to_com_port(serD,"netinfo")    


#######################################################################################
# 
#   Main
#
#######################################################################################

    
# COM-Port-Informationen abrufen
com_ports = list(serial.tools.list_ports.comports())

# Dictionary zum Zuordnen von Serialnummern zu COM-Ports erstellen
com_port_serial_dict = {}
for port_info in com_ports:
    if port_info.serial_number:
        com_port_serial_dict[port_info.device] = port_info.serial_number
        
# GUI erstellen
root = tk.Tk()
root.title("COM Port GUI")

# Das Fenster in der Größe veränderbar machen
root.geometry("1500x400")  # Startgröße des Fensters

# Frame für COM-Port-Eingabefelder und Buttons
com_port_frame = tk.Frame(root)
com_port_frame.pack(pady=10, padx=10, fill=tk.X)

com_port_status = tk.Frame(root)
com_port_status.pack(pady=10, padx=10, fill=tk.X)


#############################################################################################
# Connect Disconnet
connect_button = tk.Button(com_port_frame, text="Connect", command=connect_to_com_ports)
connect_button.pack(side=tk.LEFT)
disconnect_button = tk.Button(com_port_frame, text="Disconnect", command=disconnect_from_com_ports, state=tk.DISABLED)
disconnect_button.pack(side=tk.LEFT)
#############################################################################################




#############################################################################################
# COM-Port A Eingabefeld und Button
com_port_label_A = tk.Label(com_port_frame, text="COM-Port A:")
com_port_label_A.pack(side=tk.LEFT)
com_port_entry_A = tk.Entry(com_port_frame)
com_port_entry_A.insert(0, default_com_port_A)
com_port_entry_A.pack(side=tk.LEFT)

# COM-Port B Eingabefeld und Button
com_port_label_B = tk.Label(com_port_frame, text="COM-Port B:")
com_port_label_B.pack(side=tk.LEFT)
com_port_entry_B = tk.Entry(com_port_frame)
com_port_entry_B.insert(0, default_com_port_B)
com_port_entry_B.pack(side=tk.LEFT)

# COM-Port C Eingabefeld und Button
com_port_label_C = tk.Label(com_port_frame, text="COM-Port C:")
com_port_label_C.pack(side=tk.LEFT)
com_port_entry_C = tk.Entry(com_port_frame)
com_port_entry_C.insert(0, default_com_port_C)
com_port_entry_C.pack(side=tk.LEFT)

# COM-Port D Eingabefeld und Button
com_port_label_D = tk.Label(com_port_frame, text="COM-Port D:")
com_port_label_D.pack(side=tk.LEFT)
com_port_entry_D = tk.Entry(com_port_frame)
com_port_entry_D.insert(0, default_com_port_D)
com_port_entry_D.pack(side=tk.LEFT)
#############################################################################################




###################################################################################################
# Kommando Buttons
#
# Erstelle einen Button zum Leeren des Textfelds
clear_button_left = tk.Button(com_port_frame, text="Clear Windows", command=clear_text)
clear_button_left.pack(side=tk.LEFT)

# Erstelle einen Button iperf server
send_left_command = tk.Button(com_port_frame, text="Iperf Server", command=send_left_command_func)
send_left_command.pack(side=tk.LEFT)

# Erstelle einen Button iperf client
send_right_command = tk.Button(com_port_frame, text="Iperf Client", command=send_right_command_func)
send_right_command.pack(side=tk.LEFT)

send_netinfo = tk.Button(com_port_frame, text="netinfo", command=send_netinfo_func)
send_netinfo.pack(side=tk.LEFT)
###################################################################################################

###################################################################################################
# Button zum Starten des Python-Programms "terminal.pyw" oben rechts hinzufügen
start_button = tk.Button(com_port_frame, text="Show COM Ports", command=start_terminal_program)
start_button.pack(side=tk.LEFT)  # Mit 'anchor="ne"' wird der Button oben rechts platziert
###################################################################################################



###################################################################################################
#
# Label für COM-Port 1 erstellen und Serialnummer anzeigen
com_port_label_A = tk.Label(com_port_status, text=f"COM-Port A: {'?'}")
com_port_label_A.pack(side=tk.LEFT)

# Weitere Labels für andere COM-Ports erstellen
com_port_label_B = tk.Label(com_port_status, text=f"COM-Port B: {'?'}")
com_port_label_B.pack(side=tk.LEFT)

# Label für COM-Port 1 erstellen und Serialnummer anzeigen
com_port_label_C = tk.Label(com_port_status, text=f"COM-Port C: {'?'}")
com_port_label_C.pack(side=tk.LEFT)

# Weitere Labels für andere COM-Ports erstellen
com_port_label_D = tk.Label(com_port_status, text=f"COM-Port D: {'?'}")
com_port_label_D.pack(side=tk.LEFT)
###################################################################################################




###################################################################################################
# Erstes Textfeld für COM3-Ausgabe
text_widget_A = scrolledtext.ScrolledText(root, width=40, height=15)
text_widget_A.tag_configure("green_on_black", foreground="light green", background="black", font=("Helvetica", 7, "bold"))
text_widget_A.pack(side=tk.LEFT, padx=10, pady=10, fill=tk.BOTH, expand=True)

# Zweites Textfeld für COM4-Ausgabe
text_widget_B = scrolledtext.ScrolledText(root, width=40, height=15)
text_widget_B.tag_configure("green_on_black", foreground="light green", background="black", font=("Helvetica", 7, "bold"))
text_widget_B.pack(side=tk.LEFT, padx=10, pady=10, fill=tk.BOTH, expand=True)

# Text-Widget für weitere Ausgabe (z.B., COM5)
text_widget_C = scrolledtext.ScrolledText(root, width=40, height=15)
text_widget_C.tag_configure("green_on_black", foreground="light green", background="black", font=("Helvetica", 7, "bold"))
text_widget_C.pack(side=tk.LEFT, padx=10, pady=10, fill=tk.BOTH, expand=True)

# Text-Widget für weitere Ausgabe (z.B., COM6)
text_widget_D = scrolledtext.ScrolledText(root, width=40, height=15)
text_widget_D.tag_configure("green_on_black", foreground="light green", background="black", font=("Helvetica", 7, "bold"))
text_widget_D.pack(side=tk.LEFT, padx=10, pady=10, fill=tk.BOTH, expand=True)
###################################################################################################





# Event-Bindung für das Texteingabefeld
text_widget_A.bind("<Return>", lambda event, text_widget=text_widget_A: send_to_com_port(serA, text_widget.get("insert linestart", "insert lineend")))
text_widget_B.bind("<Return>", lambda event, text_widget=text_widget_B: send_to_com_port(serB, text_widget.get("insert linestart", "insert lineend")))
text_widget_C.bind("<Return>", lambda event, text_widget=text_widget_C: send_to_com_port(serC, text_widget.get("insert linestart", "insert lineend")))
text_widget_D.bind("<Return>", lambda event, text_widget=text_widget_D: send_to_com_port(serD, text_widget.get("insert linestart", "insert lineend")))


# Erstelle ein Textfeld zum Anzeigen der Informationen
#info_text = scrolledtext.ScrolledText(root, width=50, height=60)
# info_text.grid(row=0, column=0, columnspan=2)
#info_text.configure(font=("Helvetica", 12, "bold"), bg="black", fg="light green")  # Hintergrund und Vordergrund setzen


root.mainloop()
