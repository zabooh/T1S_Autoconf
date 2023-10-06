import os
from elftools.elf.elffile import ELFFile

ELF_FILE_01 = "..\\apps\\tcpip_iperf_lan867x\\firmware\\tcpip_iperf_lan867x_freertos.X\\dist\\FreeRTOS\\production\\tcpip_iperf_lan867x_freertos.X.production.elf"

def is_valid_function_name(name):
    # Filtere ungültige Namen wie "$t" und "$d" aus
    return not name.startswith('$')

def list_functions_and_addresses(elf_file_path, output_file_path):
    functions = []
    with open(elf_file_path, 'rb') as f:
        elf_file = ELFFile(f)
        for section in elf_file.iter_sections():
            if section.name.startswith('.text'):
                symbols = elf_file.get_section_by_name('.symtab').iter_symbols()
                prev_address = None
                for symbol in symbols:
                    if symbol.entry.st_value >= section['sh_addr'] and \
                            symbol.entry.st_value < section['sh_addr'] + section['sh_size']:
                        function_name = symbol.name
                        if function_name and is_valid_function_name(function_name):
                            if prev_address is not None:
                                # Berechne die Größe der Funktion als absolute Differenz zu vorheriger Adresse
                                function_size = abs(symbol.entry.st_value - prev_address)
                                print(function_name + "                             ", end="\r")     
                                functions.append((function_name, symbol.entry.st_value, function_size))
                            prev_address = symbol.entry.st_value

    # Sortiere die Funktionen nach ihrer Größe
    #functions = sorted(functions, key=lambda x: x[2])
    # Sortiere die Funktionen nach ihrer Größe in absteigender Reihenfolge
    functions = sorted(functions, key=lambda x: x[2], reverse=True)


    # Öffne die Ausgabedatei im Schreibmodus
    with open(output_file_path, "w") as output_file:
        for function_name, address, size in functions:
            # Schreibe die formatierte Zeile in die Ausgabedatei
            output_line = f"{address:010X}  {size:10}  {function_name}\n"
            output_file.write(output_line)

if __name__ == "__main__":
    elf_file_path = ELF_FILE_01  # Geben Sie den Pfad zur ELF-Datei ein
    output_file_path = "function_size.txt"  # Dateipfad für die Ausgabedatei    
    list_functions_and_addresses(elf_file_path, output_file_path)
