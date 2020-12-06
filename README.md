TG18-AI
=======

![screenshot](/screenshot.png)

Never used, because I couldn't get a properly working Windows build in time, so I ended up using droidbattles instead.


TODO
====

Learn math and fix this
-----------------------

![issue](/issue.png)



Local multiplayer
-----------------

 - Local LAN UDP broadcast
 - Multiplayer mode instead of just spectator mode
     - Limit local visibility
     - Send commands
 - Don't need to care about latency on LAN, just do lockstep blah blah.
 - Low priority:
     - Dedicated server handling visibility, to avoid cheating. Needs headless etc.
 - Config file support, save nick
 - Make (new) start/home screen:

```
 -----------------------------------
|        [my name]                  |
|                                   |
|        Lobby [enable]             |
|     - Player 1 [kick]             |
|     - Player 2 [kick]             |
|        [start game]               |
|                                   |
|                                   |
|       LAN Lobbies:                |
|        - [join] Foo's game        |
|        - [join] Bar's game        |
|                                   |
 -----------------------------------
```
