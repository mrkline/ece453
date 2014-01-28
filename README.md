# ECE 453 Project

Matt Kline, James Gordon, Tyler Kuske, and Creighton Long

## Overview

The proposed project is a virtual shooting range.
The setup would consist of multiple targets which could be placed around a room.
Each target would consist of an IR receiver as well as some LEDs,
and would be connected to our zedboard.
Multiple players would then have some sort of "gun" (perhaps a modified airsoft gun)
consisting of an IR transmitter and some cheap microprocessor.
Each gun would be configured to emit a unique IR sequence when the trigger is pulled,
which the zedboard/target setup would use to determine which player hit a target.
From there, it's a matter of software and several games
(timing drills where the first player to shoot a random target that lights up, multiple targets etc.)
could be played between the players. Stats could be displayed and tracked, etc.

If additional complexity is needed, the targets could be made to communicate via wireless,
or a solenoid of some sort could be added to the gun in order to simulate recoil.

## Project baselines

Most of these are just Matt spitballing. Feel free to complain to Matt if you disagree with them
or have a better idea.

### Hardware baselines

TBD

### Software baselines

- The main application that communicates with the hardware (and firmware) will be done in C++11
  as a command-line program. Communication method with the hardware is TBD. Any GUI we may add will be done
  as a separate program.

  Rationale:

  - C++, while not the simplest language by far, has a nice blend of high level niceties, execution speed,
    many freely available libraries, and ease with working on low levels directly with hardware.
    The new C++11 standard cleans up a lot of the old kruft C++ is known for.
  - Command line programs are arguably easier to compile than GUI ones
    (since GUI libraries are often large or come with many dependencies).
  - Keeping the core functionality and the GUI in separate programs allows for flexibility
    and focuses the design goals of each program.
    (The core program should focus on core functionality and exposing it in a clean way while the GUI
    should focus on easily plugging into this core program.)

- The core program will communicate with the GUI program via [JSON](http://en.wikipedia.org/wiki/JSON) over
  [UDP](http://en.wikipedia.org/wiki/User_Datagram_Protocol).

  Rationale:

  - UDP is a lightweight, simple way to communicate between processes
    and allows us to have the UI and the main application run on different machines.
  - JSON is simple and human-readable, which should be greatly useful when debugging.
    If performance becomes a problem or parsing ends up taking too much work, [BSON](http://en.wikipedia.org/wiki/BSON)
    or a binary format could be used instead.

- [Stroustrup style](http://en.wikipedia.org/wiki/Indent_style#Variant:_Stroustrup) will be used when formatting code.

  Rationale:

  - Stroustrup style is the same as Java style (braces on the same line as the preceding code)
    except for functions, which give their braces their own lines. This is because constructors may use
    [initializer lists](http://en.wikipedia.org/wiki/Initializer_list#Initializer_list) to initialize
    member variables, and putting the opening brace for the function at the end of the list may make it hard to find.
  - This is the same style as the one used in *The C++ Programming Language*, which may be used as a reference
    by group members newer to C++.

- [Git](http://git-scm.com/) will be used for version control.

  Rationale:

  - Git is an extremely popular version control system, and is available on all major PC operating systems.
  - Git is distributed, which allows group members to work individually on components before syncing up
    via `git push`, `git fetch`/`git pull`, etc.

- [Doxygen](http://www.stack.nl/~dimitri/doxygen/) will be used for documenting the code.

  Rationale:

  Doxygen is the de facto standard for documenting C++ and allows you to generate documentation from
  comments. It's like Javadoc, but better.

- Trailing whitespace will be avoided.

  Rationale:

  - Trailing whitespace can cause superfluous changes to the code to show up in version control,
    causing merge difficulties and general confusion.
  - Trailing whitespace can be easily removed with simple scripts, such as the one-liner `sed -i 's/[ \t]*$//'`.

- Unix newlines will be used instead of Windows ones.

  Rationale:

  - This is a largely arbitrary decision, but is made for the sake of consistency.
    It would be silly to have different line endings on different files.
  - Any text editor worth its salt
    (Vim, Emacs, Visual Studio, Eclipse, Notepad++, and basically anything else that isn't Notepad)
    can work fine with text files containing Unix newlines. Most of these editors also have the ability
    to convert newlines.
  - The program [dos2unix](http://sourceforge.net/projects/dos2unix/)
    can be used in case your editor does not have such conversion utilities.

## License

See `LICENSE.md`
