# Ground-support telemetry logger for the AR Nav prototype.
# Windows only. stdlib-only: ctypes -> kernel32.dll. No pyserial, no pip installs.
# NOT part of the in-flight data path - the Arduino drives the OLED standalone.
import ctypes as ct
from ctypes import wintypes as w
import sys, time

k32 = ct.WinDLL("kernel32", use_last_error=True)
GENERIC_READ, GENERIC_WRITE, OPEN_EXISTING = 0x80000000, 0x40000000, 3
DTR_ENABLE, RTS_ENABLE = 1, 1

class DCB(ct.Structure):
    _fields_ = [
        ("DCBlength", w.DWORD), ("BaudRate", w.DWORD),
        ("fBinary", w.DWORD, 1), ("fParity", w.DWORD, 1),
        ("fOutxCtsFlow", w.DWORD, 1), ("fOutxDsrFlow", w.DWORD, 1),
        ("fDtrControl", w.DWORD, 2), ("fDsrSensitivity", w.DWORD, 1),
        ("fTXContinueOnXoff", w.DWORD, 1), ("fOutX", w.DWORD, 1),
        ("fInX", w.DWORD, 1), ("fErrorChar", w.DWORD, 1),
        ("fNull", w.DWORD, 1), ("fRtsControl", w.DWORD, 2),
        ("fAbortOnError", w.DWORD, 1), ("fDummy2", w.DWORD, 17),
        ("wReserved", w.WORD), ("XonLim", w.WORD), ("XoffLim", w.WORD),
        ("ByteSize", ct.c_ubyte), ("Parity", ct.c_ubyte), ("StopBits", ct.c_ubyte),
        ("XonChar", ct.c_char), ("XoffChar", ct.c_char), ("ErrorChar", ct.c_char),
        ("EofChar", ct.c_char), ("EvtChar", ct.c_char), ("wReserved1", w.WORD),
    ]

class COMMTIMEOUTS(ct.Structure):
    _fields_ = [("ReadIntervalTimeout", w.DWORD), ("ReadTotalTimeoutMultiplier", w.DWORD),
                ("ReadTotalTimeoutConstant", w.DWORD), ("WriteTotalTimeoutMultiplier", w.DWORD),
                ("WriteTotalTimeoutConstant", w.DWORD)]

def open_port(name, baud):
    h = k32.CreateFileW("\\\\.\\" + name, GENERIC_READ | GENERIC_WRITE,
                         0, None, OPEN_EXISTING, 0, None)
    if h in (0, -1):
        raise OSError(f"CreateFileW failed for {name}, GetLastError={ct.get_last_error()}")
    dcb = DCB(); dcb.DCBlength = ct.sizeof(DCB)
    if not k32.GetCommState(h, ct.byref(dcb)):
        raise OSError("GetCommState failed")
    dcb.BaudRate, dcb.ByteSize, dcb.Parity, dcb.StopBits = baud, 8, 0, 0
    dcb.fBinary, dcb.fParity, dcb.fDtrControl, dcb.fRtsControl = 1, 0, DTR_ENABLE, RTS_ENABLE
    if not k32.SetCommState(h, ct.byref(dcb)):
        raise OSError("SetCommState failed")
    to = COMMTIMEOUTS(50, 0, 200, 0, 0)  # 200ms overall read timeout, non-blocking-ish
    k32.SetCommTimeouts(h, ct.byref(to))
    return h

def read_line(h):
    line, buf, n = bytearray(), ct.create_string_buffer(1), w.DWORD(0)
    while True:
        ok = k32.ReadFile(h, buf, 1, ct.byref(n), None)
        if not ok or n.value == 0:
            return None  # timeout / no data yet
        if buf.raw[0:1] == b"\n":
            return line.decode(errors="ignore").strip()
        line += buf.raw[0:1]

if __name__ == "__main__":
    port = sys.argv[1] if len(sys.argv) > 1 else "COM5"
    baud = int(sys.argv[2]) if len(sys.argv) > 2 else 115200
    h = open_port(port, baud)
    print(f"Logging {port} @ {baud} baud. Ctrl+C to stop.")
    fname = f"telemetry_{time.strftime('%Y%m%d_%H%M%S')}.csv"
    try:
        with open(fname, "a") as f:
            f.write("lat,lon,speed_kmh,course_deg,pitch_deg,roll_deg,gps_fix\n")
            while True:
                ln = read_line(h)
                if ln:
                    print(ln)
                    f.write(ln + "\n"); f.flush()
    except KeyboardInterrupt:
        pass
    finally:
        k32.CloseHandle(h)
