import os
import sys

src = sys.argv[1]
dest = src + ".h"

title = """
#pragma once
inline unsigned char {}[]
{{
{}
}};
"""

with open(dest, 'w') as d:
    with open(src, 'rb') as s:

        bytes = s.read()
        contents = str()
        for i in range(len(bytes)):
            if i % 8 == 0:
                contents += "\n    ";
            contents += "0x{:02X}, ".format(bytes[i])
        
        fieldname = 'FILE_' + os.path.basename(src).replace('-','_').replace('.', '_')
        write = title.format(fieldname , contents)
        d.write(write)
            
        