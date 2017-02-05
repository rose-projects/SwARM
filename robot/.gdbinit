target ext :2331
mon endian little
mon halt
load
mon reset

# User interface with asm, regs and cmd windows
define split
  layout split
  layout asm
  layout regs
  focus cmd
end

# Remove annoying quit anyway question
define hook-quit
    set confirm off
end

# User defined shortcut for target init define flash
define flash
    dont-repeat
    mon halt
    mon reset
    mon reset
    load
    b main
    cont
end
