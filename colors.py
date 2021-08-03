# 256-colored backgrounds work too:

# The 8-bit colours are arranged like so:
#
# 0x00-0x07:  standard colors (same as the 4-bit colours)
# 0x08-0x0F:  high intensity colors
# 0x10-0xE7:  6 × 6 × 6 cube (216 colors): 16 + 36 × r + 6 × g + b (0 ≤ r, g, b ≤ 5)
# 0xE8-0xFF:  grayscale from black to white in 24 steps

# \033[38;2;<r>;<g>;<b>m     #Select RGB foreground color
# \033[48;2;<r>;<g>;<b>m     #Select RGB background color

import sys

import sys
for i in range(0, 32):
    for j in range(0, 8):
        code = str(i * 8 + j)
        sys.stdout.write(u"\u001b[38;5;" + code + "m " + code.ljust(4))
    print(u"\u001b[0m")

print("\n")

for i in range(0, 32):
    for j in range(0, 8):
        code = str(i * 8 + j)
        sys.stdout.write(u"\u001b[48;5;" + code + "m " + code.ljust(4))
    print(u"\u001b[0m")
