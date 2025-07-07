from reaper_python import *
import serial
import time
import math

SERIAL_PORT = '/dev/tty.usbserial-83420'
BAUD_RATE   = 115200

serial_conn     = None
buffer          = ""
last_sent_msg   = ""
selected_track  = 0

def db_to_linear(db):
    return round(10 ** (db / 20.0), 6)

def linear_to_db(vol):
    if vol <= 0.00001:
        return -60.0
    return 20 * math.log10(vol)

def db_to_midi(db):
    if db <= -60.0:
        return 0
    elif db >= 12.0:
        return 127
    return round((db + 60) / 72 * 127)

def pan_to_midi(pan):
    return round((pan + 1.0) * 63.5)

def apply_setvol(cmd):
    delta = int(cmd.split(":")[1])
    track = RPR_GetTrack(0, selected_track)
    vol = RPR_GetMediaTrackInfo_Value(track, "D_VOL")
    db  = linear_to_db(vol)
    db += delta * 0.75
    db  = max(-60.0, min(12.0, db))
    RPR_SetMediaTrackInfo_Value(track, "D_VOL", db_to_linear(db))

def apply_setpan(cmd):
    delta = int(cmd.split(":")[1])
    track = RPR_GetTrack(0, selected_track)
    pan = RPR_GetMediaTrackInfo_Value(track, "D_PAN")
    pan += delta * 0.02
    pan  = max(-1.0, min(1.0, pan))
    RPR_SetMediaTrackInfo_Value(track, "D_PAN", pan)

def apply_jog(cmd):
    global selected_track
    delta = int(cmd.split(":")[1])
    track_count = RPR_CountTracks(0)
    selected_track = (selected_track + delta) % track_count
    RPR_ShowConsoleMsg(f"[JOG] New selected track: {selected_track}\n")

def send_track_info():
    global last_sent_msg
    track = RPR_GetTrack(0, selected_track)
    _,_,_,name,_ = RPR_GetSetMediaTrackInfo_String(track, "P_NAME", "", False)
    name      = name.encode('ascii','ignore').decode()
    vol       = RPR_GetMediaTrackInfo_Value(track, "D_VOL")
    pan       = RPR_GetMediaTrackInfo_Value(track, "D_PAN")
    db        = linear_to_db(vol)
    vm, pm    = db_to_midi(db), pan_to_midi(pan)
    msg       = f"NAME:{name}|VOL:{vm}|PAN:{pm}\n"
    if msg != last_sent_msg:
        serial_conn.write(msg.encode('ascii'))
        last_sent_msg = msg

def apply_command(cmd):
    if cmd.startswith("SETVOL:"):
        apply_setvol(cmd)
    elif cmd.startswith("SETPAN:"):
        apply_setpan(cmd)
    elif cmd.startswith("JOG:"):
        apply_jog(cmd)

def init_serial():
    global serial_conn
    serial_conn = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.01)
    time.sleep(2)
    RPR_ShowConsoleMsg("[INFO] Serial connected.\n")

def loop():
    global buffer
    if serial_conn is None:
        init_serial()
        RPR_defer("loop()")
        return

    try:
        while serial_conn.in_waiting:
            c = serial_conn.read().decode('ascii', 'ignore')
            if c == '\n':
                line = buffer.strip()
                if line:
                    apply_command(line)
                buffer = ""
            else:
                buffer += c

        send_track_info()

    except Exception as e:
        RPR_ShowConsoleMsg(f"[ERROR LOOP] {e}\n")

    RPR_defer("loop()")

loop()
