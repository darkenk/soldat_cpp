#!/usr/bin/python
# release mouse when breaking into debugger for SDL2 programs
# by KittyCat, some comments + gdb.write() messages by Daniel Gibson
# put this next to the executable you wanna debug and name it
# <YourExecutableName>-gdb.py
# Make sure that it's in a subdir of an allowed GDB auto-loading safe-path,
# see https://sourceware.org/gdb/onlinedocs/gdb/Auto_002dloading-safe-path.html
# e.g. by executing in a terminal:
# $ echo "add-auto-load-safe-path /path/to/project" >> ~/.gdbinit

def release_mouse (event):
    gdb.write("GDB/SDL2: Releasing mouse\n")
    gdb.execute("call (int)SDL_SetRelativeMouseMode(0)")
    pass
     
    # TODO: if you use SDL_SetWindowGrab(), you may have to write
    #       function with no arguments to call it with your SDL_Window*
    #       handle and call this as well, like:
    #gdb.execute("call DEBUG_UngrabMouse()")
        
    # TODO: the following doesn't seem to work, maybe it does for you.
    #       either way, it needs xdotool installed.
    #gdb.execute("exec xdotool key XF86Ungrab")
     
#gdb.events.stop.connect(release_mouse)
#gdb.write("GDB/SDL2: installed release mouse for SDL2\n")
