# Converting graphics to ANSI colors:

Smallest program - no issues compile!
- https://github.com/stolk/imcat


# Unicode characters to use

Unicode characters as 8x8 font!

https://docs.rs/font8x8/0.1.0/font8x8/

- braille 8x8 spacious ⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿
  U+2800 to U+28ff (1-8/2-16/4-32/64-128)

- teletext G1 legacy computing (not so often in fonts) https://unicode-table.com/en/blocks/symbols-for-legacy-computing/

- https://github.com/termux/termux-app/issues/142

- teletext G3 some thin box-drawing characters https://en.m.wikipedia.org/wiki/Teletext_character_set
- ansi block elements (half/full - not complete) and 2x2 pixels, shaded full elements
  https://en.m.wikipedia.org/wiki/Block_Elements
- triangles on square basis
  https://en.m.wikipedia.org/wiki/Geometric_Shapes

Unicode block graphics font mapped to pixels converter of image .cpp
jsk: works fine, but 1 char=1 pixel
- https://github.com/stefanhaustein/TerminalImageViewer/blob/master/src/main/cpp/tiv.cpp

can do gif/movies/streamed
- https://github.com/danielgatis/imgcat

