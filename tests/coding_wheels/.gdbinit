target ext :2331
mon endian little
mon halt

# User defined shortcut for target init
define flash
    dont-repeat
    mon halt
    mon reset
    mon reset
    load
    b main
    cont
end

# User interface with asm, regs and cmd windows
define split
layout split
layout asm
layout regs
focus cmd
end
