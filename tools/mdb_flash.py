

import subprocess
import sys
import re
import os.path
import argparse
import time

try:
    reload(sys)
    sys.setdefaultencoding('utf8')
except:
    pass


###########################################################################
#    Parse Arguments 
###########################################################################

parser = argparse.ArgumentParser()

parser.add_argument('--hex',
    required = True,
    help = 'Path to hex file to program'
    )

parser.add_argument('--mdb-path',
    required = True,
    help = 'Path to mdb.bat (mdb.sh on Linux).'
    + 'Typical location on Linux is: "/opt/microchip/mplabx/vX.XX/mplab_platform/bin/mdb.sh"'
    )

parser.add_argument('--mcu',
    required = True,
    help = 'MCU type to program. For example: "PIC18F87K90"'
    )

parser.add_argument('--hwtool',
    help = 'Hardware tool to use. Possible values are: "PICkit3", "ICD3", "ICD4", "RealICE", "SIM", "PM3".'
    )

parser.add_argument('--hwtool-serial',
    help = 'If there are several hardware tools attached, you need to specify the serial number of one to use. Example: "JIT140210129"'
    )

parser.add_argument('--voltage',
    default='',
    help = 'Power supply voltage of the device. Example: --voltage="5.0"'
    )

parser.add_argument('--powerenable',
    default=False,
    action='store_true',
    help = 'Programming without external power supply, required --voltage. example: --powerenable --voltage="3.3"'
    )

myargs = parser.parse_args()


###########################################################################
# Function: send_quit_and_exit(code)     
#   exit the program in case of an exception 
###########################################################################
def send_quit_and_exit(code):
  """
  Should be used to prematurely exit when the mdb is running: it issues
  the 'quit' command to mdb, causing it to quit, and then calls
  sys.exit() with given code.
  """

  #-- when we issue the "quit" command, mdb.sh will exit, causing
  #   mdb_communicator to exit, which causes StopIteration exception.
  try:
    out = m.send('quit\n')
  except StopIteration:
    print('mdb finished')

  sys.exit(code)


###########################################################################
# Function: mdb_communicator(proc)     
#   worker thread for communication
###########################################################################
def mdb_communicator(proc): # {{{
  """
  The coroutine that communicates with the mdb. It yields every time the mdb
  returns ">" in the beginning of the line, indicating that it waits for the
  user input. The value yielded is the string after previous ">".

  The client should send() next string to send to mdb.

  @param proc
    The process of mdb
  """

  string = ""
  line = ""

  while True:
    if proc.poll() != None: break

    #-- get next byte from mdb's stdout
    byte = proc.stdout.read(1)
    if byte != '':

      if (line == ""):
        sys.stdout.write("[mdb out] ")

      string += byte.decode('UTF-8')
      line += byte.decode('UTF-8')

      sys.stdout.write(byte.decode('UTF-8'))

      if (byte == '\n'):
        line = ""
      if ((len(string) < 2 or string[-2] == '\n') and string[-1] == '>'):
        #-- the ">" in the beginning of the line: mdb waits for user input.
        #   yield to caller
        if string == '' : break
        to_send = (yield string)

        #-- reset current string and line
        string = ""
        line = ""

        #-- echo to terminal
        sys.stdout.write("\n[send] " + to_send)

        #-- send to mdb what the caller asked
        proc.stdin.write(to_send.encode('utf-8'))
        proc.stdin.flush()

    else:
      sys.stdout.write("finishing\n")
      break



###########################################################################
# Function: get_index_by_serial(data, serial):     
#   find index of tool by serial number
###########################################################################
import re
def get_index_by_serial(data, serial):
    lines = data.split('\n') 
    for line in lines:
        match = re.match(r'\s*(?P<index>\d+)\s+\S+\s+(?P<serial>[^\s]+)', line)
        if match:
            index = match.group('index')
            extracted_serial = match.group('serial')
            if extracted_serial == serial:
                return index
    return None





###########################################################################
# Main Code
###########################################################################
if not os.path.isfile(myargs.mdb_path):
  print('Given mdb-path "' + myargs.mdb_path + '" is invalid. Typical location on Linux is: "/opt/microchip/mplabx/vX.XX/mplab_ide/bin/mdb.sh"')
  sys.exit(1)

if not os.path.isfile(myargs.hex):
  print('No such hex file: "' + myargs.hex + '".')
  sys.exit(1)

if myargs.mcu == None:
  print('No mcu was specified')
  sys.exit(1)



#-- run mdb as a child process, and pipe its stdin / stdout / stderr.
proc = subprocess.Popen(
        [myargs.mdb_path],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE
        )

question_pattern = re.compile(
    ".*Do you .* to continue\\?",
    re.MULTILINE | re.DOTALL | re.IGNORECASE
    )






#-- start communication with mdb
m = mdb_communicator(proc)

#-- wait until mdb asks for the user input first time
out = next(m)

#-- get list of all hardware tools attached
out = m.send('hwtool\n')

#-- get the index by given serial number (or '0' if no serial is given,
#   and there is just one tool attached)
# hwtool_index = get_hwtool_index_by_serial(myargs.hwtool, myargs.hwtool_serial, out)

hwtool_index = get_index_by_serial(out, myargs.hwtool_serial)
print("Found hwtool_index " + hwtool_index + " for Serial " + myargs.hwtool_serial + "\n")

#-- check if hwtool index was found
if (hwtool_index == None):
  print("No hwtool available")
  send_quit_and_exit(1)


#-- select mcu
out = m.send('device ' + myargs.mcu + '\n')

#-- for some reason, pickit3 doesn't program EEPROM in 'auto' mode
#   (probably it's just when EEPROM is not empty, didn't check)
#
#   So here we just enable it explicitly
out = m.send('set AutoSelectMemRanges auto\n')
# out = m.send('set memories.eeprom true\n')

out = m.send('set communication.interface swd\n')
out = m.send('set communication.speed 6.000\n')

if myargs.powerenable:
  #-- power supply voltage of the device
  out = m.send('set voltagevalue '+myargs.voltage+'\n')
  #-- for programming without external power supply of the device
  out = m.send('set poweroptions.powerenable true\n')

#-- select hwtool by index (obtained earlier)
out = m.send('hwtool ' + myargs.hwtool + ' -p ' + hwtool_index + '\n')

#-- if there is some question like "do you want to continue?", say yes
if question_pattern.match(out):
  out = m.send('yes\n')

#-- program firmware
out = m.send('program "' + myargs.hex + '"\n')

#-- check if programming was successful
#-- (by default, result indicates an error)
result = 1
if (
    re.search(
      'Programming\/Verify complete', out, re.MULTILINE | re.DOTALL | re.IGNORECASE
      )
    and
    re.search(
      'Program\s+succeeded', out, re.MULTILINE | re.DOTALL | re.IGNORECASE
      )
    ):
  result = 0

m.send('reset\n')

#-- NOTE: this sleep is necessary for PICkit3: without it, the EEPROM
#   is not yet finished programming when mdb reports "Program succeeded",
#   and if we type "quit" immediately, the EEPROM memory is corrupted.
#
#   With this hacky sleep, it works.
time.sleep(1)

send_quit_and_exit(result)








###########################################################################
# Function: mdb_communicator(proc)     
#   worker thread for communication
###########################################################################
#def get_hwtool_index_by_serial(hwtool, serial, mdb_output): # {{{
#  """
#  Tries to retrieve index of the attached hardware tool by its serial number.
#  If the tool is found, string like "0", "1", etc, is returned.
#  Otherwise, None is returned.###
#
# If the serial is empty, and there is just one hardware tool attached, then
#  "0" is returned.
#
# @param serial
#    Serial number: string like "JIT140210129"
#  @param mdb_output
#    The output returned from mdb_communicator after sending "hwtool" to it.
#  """
#
#  hwtool_name_pattern = ''
#
#  if hwtool == 'ICD3':
#    hwtool_name_pattern = 'MPLAB\s+ICD3\s+tm\s*'
#  elif hwtool == 'PICkit3':
#    hwtool_name_pattern = 'PICkit\s*3\s*'
#  elif hwtool == 'ICD4':
#    hwtool_name_pattern = 'MPLAB\s+ICD\s*4\s*'
#  elif hwtool == 'EDBG':
#    hwtool_name_pattern = '\s*edbg\s*'
#
#
#  hwtool_pattern = re.compile(
#    r'^\s*(?P<index>\d+)\s+\s*edbg\s*\((?P<serial>[^)]+)\)',
#    re.IGNORECASE | re.MULTILINE | re.DOTALL
#  )
#
#  print("hwtool_name_pattern:", hwtool_name_pattern)
#  print("hwtool_pattern:", hwtool_pattern)
# 
#  print("mdb_output:")   
#  print(mdb_output)
#
#  matches = [m.groupdict() for m in hwtool_pattern.finditer(mdb_output)]
#  print("matches:")
#  print(matches)
#    
#  ret = None
#
#  if (serial == None and len(matches) == 1):
#    # no serial was given, and only one hwtool is attached: just use it
#    ret = matches[0]['index']
#  else:
#    for item in matches:
#      if (item['serial'] == serial):
#        ret = item['index']
#        break;
#
#  return ret
###########################################################################
# }}}

