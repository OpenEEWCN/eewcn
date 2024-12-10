# python 3.10

import sys
import base64

revshift = "LXFY"

if len(sys.argv) < 2:
    print("qmake_muuid.py <machine_id>")
else:
    mid = sys.argv[1]
    uuidbytes = base64.b64decode(mid)
    muuid = ""
    for i in range(0,len(uuidbytes),1):
        muuid = muuid + "%02X"%((int(uuidbytes[i])+256-ord(revshift[i%4]))%256)
    f = open("qmake_muuid.txt","w")
    print(muuid)
    f.write(muuid)
