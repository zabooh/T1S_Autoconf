import os
from elftools.elf.elffile import ELFFile

ELF_FILE_01 = "..\\apps\\tcpip_iperf_lan867x\\firmware\\tcpip_iperf_lan867x_freertos.X\\dist\\FreeRTOS\\production\\tcpip_iperf_lan867x_freertos.X.production.elf"

def is_valid_symbol_name(name):
    # Filtere ungültige Namen wie "$t" und "$d" aus
    return not name.startswith('$') and not name.startswith('c:/') and name.strip() != ''

def list_symbols_and_addresses(elf_file_path, output_file_path):
    symbols_list = []  # Verwende einen anderen Namen für die Liste
    with open(elf_file_path, 'rb') as f:
        elf_file = ELFFile(f)
        for section in elf_file.iter_sections():
            if section.name.startswith('.data') or section.name.startswith('.bss'):
                symbols = elf_file.get_section_by_name('.symtab').iter_symbols()
                for symbol in symbols:
                    if is_valid_symbol_name(symbol.name):
                        symbol_address = symbol.entry.st_value
                        symbol_size = symbol.entry.st_size
                        symbol_name = symbol.name
                        # Überprüfe, ob das Symbol eine Größe größer als 0 hat und nicht in der Liste ist
                        if symbol_size > 0 and (symbol_name, symbol_address, symbol_size) not in symbols_list:
                            print(f"Symbol: {symbol_name}       ", end="\r")
                            symbols_list.append((symbol_name, symbol_address, symbol_size))

    # Sortiere die Symbole nach ihren Adressen im Speicher
    symbols_list = sorted(symbols_list, key=lambda x: x[2], reverse=True)

    # Öffne die Ausgabedatei im Schreibmodus
    with open(output_file_path, "w") as output_file:
        for symbol_name, address, size in symbols_list:
            # Schreibe die formatierte Zeile in die Ausgabedatei
            output_line = f"{address:010X}  {size:10}  {symbol_name}\n"
            output_file.write(output_line)

if __name__ == "__main__":
    elf_file_path = ELF_FILE_01  # Geben Sie den Pfad zur ELF-Datei ein
    output_file_path = "symbol_location.txt"  # Dateipfad für die Ausgabedatei    
    list_symbols_and_addresses(elf_file_path, output_file_path)
