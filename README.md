A LITTLE GAME
=============

Rules
-----

It's a game for 2 players. There are 2 blue LEDs on top. Each player has 2 buttons - one for each LED. 
If you press the correct button when a LED lights up, you earn a point. If you press the wrong one, 
or do it before any LED lights up, you get 1s cooldown when you can't do anything. 
The LEDs in the middle from green to red are point tracks for both players. When you get max points + 1 you win.
When a point track goes blank, it means that the cooldown is in progress.

Disclaimer
----------

There's no nice OOP design here and no tests. There are a lot of global variables too. I'm aware it all could
be nicer but I did in my spare time and the program is short. I hope the comments make up for the lackings.

Pin mapping
-----------

```
LEFT   DIP pin  Arduino pin
blu    19       13
cp     6        4
mr     5        3
da     4        2
btnu   15       9
btnd   16       10

RIGHT  DIP pin  Arduino pin
blu    14       8
cp     13       7
mr     12       6
da     11       5
btnu   17       11
btnd   18       12
```
