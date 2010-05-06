Tomaatti
========

* For license information, see License.txt
* For instructions on compiling the game from sources, refer to Compiling.txt
* For list of authors, see Authors.txt

Command line parameters
-----------------------
--help | -h               - A brief help.
--server [port]           - Start in dedicated server mode, no window is opened.
                            If port is omitted, defaults to 1234.
--client [host] [port]    - Connect to a server for an instant multiplayer game.
                            Defaults to "localhost" and 1234.


Power-ups
---------
* Mine         - Invisible mine kills when touched
* Minigun      - Shoots (horizontally) deadly
* Double-jump  - Jump twice
* Punch        - Punches enemy with great force
* Invisibility - Turns you invisible
* Superball    - Bounces
* Low gravity  - Gravity affects very little
* Teleport     - Teleports to random (safe) location

Power-downs
-----------
* Death        - Kills
* Disease      - Reversed controls

Technical
---------
* Takes advantage of the graphics card through OpenGL.
* Takes advantage of multiple processor cores through multithreading:
	- Graphics, physics, viewport updating, input, networking are all handled in parallel.
* Uses UDP protocol for networking.
