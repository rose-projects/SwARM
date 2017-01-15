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
