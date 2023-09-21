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
default_com_port_1 = 'COM3'
default_com_port_2 = 'COM4'
baud_rate = 115200

ser1 = None  # Serial-Objekt für COM-Port 1
ser2 = None  # Serial-Objekt für COM-Port 2
    
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
    global ser1, ser2
    com_port_1 = com_port_entry_1.get()
    com_port_2 = com_port_entry_2.get()

    update_com_port_label_x()
    
    print(f"Verbindung zu COM-Port {com_port_1} und {com_port_2} wird hergestellt...")

    try:
        ser1 = serial.Serial(com_port_1, baud_rate, timeout=0)
        ser2 = serial.Serial(com_port_2, baud_rate, timeout=0)

        print(f"Verbindung zu COM-Port {com_port_1} und {com_port_2} hergestellt.")

        # Threads zum Lesen von Daten von den COM-Ports erstellen und starten
        com_reader_thread_1 = threading.Thread(target=read_from_com_port, args=(ser1, text_widget_1))
        com_reader_thread_1.daemon = True
        com_reader_thread_1.start()

        com_reader_thread_2 = threading.Thread(target=read_from_com_port, args=(ser2, text_widget_2))
        com_reader_thread_2.daemon = True
        com_reader_thread_2.start()

        # Deaktiviere die COM-Port-Eingabefelder und den Connect-Button nach der Verbindung
        com_port_entry_1.config(state=tk.DISABLED)
        com_port_entry_2.config(state=tk.DISABLED)
        connect_button.config(state=tk.DISABLED)
        disconnect_button.config(state=tk.NORMAL)

    except serial.SerialException as e:
        print(f"Fehler bei der seriellen Verbindung: {e}")



# Funktion zum Trennen der COM-Port-Verbindung
def disconnect_from_com_ports():
    global ser1, ser2
    if ser1 is not None:
        ser1.close()
    if ser2 is not None:
        ser2.close()

    # Aktiviere die COM-Port-Eingabefelder und den Connect-Button nach der Trennung
    com_port_entry_1.config(state=tk.NORMAL)
    com_port_entry_2.config(state=tk.NORMAL)
    connect_button.config(state=tk.NORMAL)
    disconnect_button.config(state=tk.DISABLED)


# Funktion zum Lesen von Daten vom COM-Port und Aktualisieren des GUI-Textfelds
def read_from_com_port(ser, text_widget):
    while True:
        try:
            data = ser.read(1)  # Ein Zeichen vom COM-Port lesen
            if data:
                process_vt100_escape(text_widget, data.decode())  # Daten im Textfeld anzeigen und VT100 Escape-Sequenzen verarbeiten
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
   com_port_label_3.config(text=f"COM-Port A: {com_port_serial_dict.get(com_port_entry_1.get(), 'Nicht verfügbar')}")    
   com_port_label_4.config(text=f"COM-Port B: {com_port_serial_dict.get(com_port_entry_2.get(), 'Nicht verfügbar')}")
    
def clear_text_left():
    text_widget_1.delete(1.0, tk.END)

def clear_text_right():
    text_widget_2.delete(1.0, tk.END)    
    
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
root.geometry("1400x400")  # Startgröße des Fensters

# Frame für COM-Port-Eingabefelder und Buttons
com_port_frame = tk.Frame(root)
com_port_frame.pack(pady=10, padx=10, fill=tk.X)

com_port_status = tk.Frame(root)
com_port_status.pack(pady=10, padx=10, fill=tk.X)

# COM-Port 1 Eingabefeld und Button
com_port_label_1 = tk.Label(com_port_frame, text="COM-Port A:")
com_port_label_1.pack(side=tk.LEFT)
com_port_entry_1 = tk.Entry(com_port_frame)
com_port_entry_1.insert(0, default_com_port_1)
com_port_entry_1.pack(side=tk.LEFT)

connect_button = tk.Button(com_port_frame, text="Connect", command=connect_to_com_ports)
connect_button.pack(side=tk.LEFT)

disconnect_button = tk.Button(com_port_frame, text="Disconnect", command=disconnect_from_com_ports, state=tk.DISABLED)
disconnect_button.pack(side=tk.LEFT)

# COM-Port 2 Eingabefeld und Button
com_port_label_2 = tk.Label(com_port_frame, text="COM-Port B:")
com_port_label_2.pack(side=tk.LEFT)
com_port_entry_2 = tk.Entry(com_port_frame)
com_port_entry_2.insert(0, default_com_port_2)
com_port_entry_2.pack(side=tk.LEFT)



# Button zum Starten des Python-Programms "terminal.pyw" oben rechts hinzufügen
start_button = tk.Button(com_port_frame, text="Show COM Ports", command=start_terminal_program)
start_button.pack(side=tk.LEFT)  # Mit 'anchor="ne"' wird der Button oben rechts platziert


# Erstelle einen Button zum Leeren des Textfelds
clear_button_left = tk.Button(com_port_frame, text="Clear Left Window ", command=clear_text_left)
clear_button_left.pack(side=tk.LEFT)

# Erstelle einen Button zum Leeren des Textfelds
clear_button_right = tk.Button(com_port_frame, text="Clear Right Window ", command=clear_text_right)
clear_button_right.pack(side=tk.LEFT)


# Label für COM-Port 1 erstellen und Serialnummer anzeigen
com_port_label_3 = tk.Label(com_port_status, text=f"COM-Port A: {'?'}")
com_port_label_3.pack(side=tk.LEFT)

# Weitere Labels für andere COM-Ports erstellen
com_port_label_4 = tk.Label(com_port_status, text=f"COM-Port B: {'?'}")
com_port_label_4.pack()





# Erstes Textfeld für COM3-Ausgabe
text_widget_1 = scrolledtext.ScrolledText(root, width=40, height=15)
text_widget_1.pack(side=tk.LEFT, padx=10, pady=10, fill=tk.BOTH, expand=True)

# Zweites Textfeld für COM4-Ausgabe
text_widget_2 = scrolledtext.ScrolledText(root, width=40, height=15)
text_widget_2.pack(side=tk.LEFT, padx=10, pady=10, fill=tk.BOTH, expand=True)



# Event-Bindung für das Texteingabefeld
text_widget_1.bind("<Return>", lambda event, text_widget=text_widget_1: send_to_com_port(ser1, text_widget.get("insert linestart", "insert lineend")))
text_widget_2.bind("<Return>", lambda event, text_widget=text_widget_2: send_to_com_port(ser2, text_widget.get("insert linestart", "insert lineend")))

root.mainloop()
