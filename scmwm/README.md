# SCMWM

## So what's this window manager?
This window manager is a floating window manager with an emphasis on simplicity in
design (everything is done through a client program), having as much of the code
in one place as possible (our only non-UNIX dependency is XCB), and being
keyboard-focused and easy to use.

## Design decisions
* Pure floating: This has 2 reasons. It's much more straightforward to implement, which
makes the code easier to read and doesn't require a tiling algorithm. The other is that
many existing floating window managers tend to be huge and bundled with more
complicated software (ex: GNOME).
* Keyboard focus: If you need a mouse to use a window manager, the window manager is
written wrong. This is because mice are imprecise and annoying, especially on laptops.
* Controlled by a client: This one is taken from herbstluftwm and hootwm. It's convenient
for building config files.

## Lower-level design (if you want to roll your own client)
* Communication is done by a well-known FIFO currently specified in a config file.
* The keyboard server: TBD.
* Currently no calls to xlib, only to xcb.

## Installation
```
$ # Under Ubuntu
$ apt install libxcb1-dev
$ # Install the window manager
$ make install
```
Then go back into your display manager or however else you start window managers.

That *should* be all you have to do, but I've had some real problems with `gdm`.

Commands can then be echoed to the FIFO, with some built in commands to allow
movement and some default programs

Code by Gabriel Miller and Sam Shippey
