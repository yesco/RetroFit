// ansi-utf8
// bansi-utf8
// butf-8
// format-utf8
//
// fansi-utf8
// fansy-utf8
// futf-8
//
// autf-8 (ansi)
// eutf-8 (extended)
// xutf-8 (eXtended)
// gutf-8 (graphic)
// jutf-8 !!! lol
// wutf-8 (too colose to wtf-8)
// dbutf-8 (database?)

// Binary ansi ?
/*


  Terminal state (4 bytes = "int")
    1 byte bg (256)
    1 byte fg (256)
    1 byte 8 bits features:
       underline bold italic blink
       overline strike reverse ?cursor
    1 byte
       4 low bits indent (0-15)
       1 is pre
       1 is skip
       1 last was whitespace
       1 last was newline

   Window state (4 bytes = "int")
    2 byte cursor (row, col)
    2 byte origin (row, col)
    2 byte size (row, col)
    2 byte ...?




  00 -- \0-terminated strings
  01 -- title     (SOH Start Of Heading)
  02 -- display      (STX Start Of Text)
  03 -- hide           (ETX End Of Text)
  04 -- underline    (EOT End Of Transm)
  05 -- bold               (ENQ Enquiry)
  06 -- italic         (ACK Acknowledge)
  07 -- off                        (BEL)

  08 -- (BS) (compose on graphical!)
  09 -- TAB (compressed 8 spaces)
  0a -- \n (end of line)
  0b -- save cursor/color (VT up)
  0c -- restore cursor/color (FF cls)
  0d -- \r (start of line)
  0e -- link ("keys url content") (SO)
  0f -- link end (SI)

  10 -- black
  11 -- red
  12 -- green
  13 -- yellow
  14 -- blue
  15 -- magenta
  16 -- cyan
  17 -- white

  18 -- none? (color)     (^Z = EOF!)
  19 -- fg-prefix (next char 256 color)
  1a -- bg-prefix (next char 256 color)
  1b -- ESC (ansi=bold/underline/italic)

  1c -- database (FS file separator)
  1d -- table sep (GS group separator)
  1e -- row sep (RS record separator)

  1f -- hard space (US - unit separator)

  7f -- DEL (?)

  // UTF-8 encoding
  0xxx xxxx ASCII              (128)
  10xx xxxx continuation UTF-8 (64)
  110x xxxx + 1 cont = 2 bytes (32)
  1110 xxxx + 2 cont = 3 bytes (16)
  1111 0xxx + 3 cont = 4 bytes (8)
  1111 1xxx ---              + (8)
                             ========
                               (256)

  ESC + P ...  g (jsk quad graphics)
  compressed quads?
    1 bit 0=fg 1=bg
    3 bits color (0=black... 7=white)
    4 bits pattern (quads)
    
  10xx xxxx sixels (=cont w/o prefix!)
            (block sextant = 64)
            (16 quads (1 is "space")
             16 block (6 ovrlp w quads)
             15 scandi (öäåÅÄÖþÞæÆøØßüÜ)
             16 graphics (bullet point?)

                box drawing?

                "─│┌┐└┘├┤┬┴┼" (12)
                "╮╯╰" (4) (soft)

                (6 extra in block!)

                "»«" (2)
                "╱╲╳" (3)
                "●" (1)

           )
             


  ESC + P p1 ; p2 ; p3 ; q (sixel-mode)
    (p1=aspect ratio (deprecated))
    (p2=how to interpret color of 0)
    (p3=grid size parameters?)

  64-127 (sixels)
  ! <count> <sixel> -- repeat!
  / CRLF (vt300 defining chars)
  # <regnum> (use color from regnum)
  # <regnum> ; 2 ; <%R> ; <%G> ; <%B>
     "set color register 0,
     use mode 2 (RGB mode),
     set R, G and B to n%" (0-100)
  ... $ (next line overprint current)
  ... - (next line is new line)

  ESC + \ ("Stop Text")
  - https://en.m.wikipedia.org/wiki/Sixel

  (illegal unicode chars or combos!)
  8 >4 bytes  (x0-x7)/8 - ???
  4 overlongs (y0-y3)/4 - ???
  4 to big    (o0-o3)/4 - ???
  1 overlongs (z0   )/1 - ???

  1100 0000 overlong ASCII! (C0) y0
  1100 0001 overlong ASCII! (C1) y1
  1100 0001 MS 2-byte null! (C1 80)
  
  1110 0000 overlong +2 cont (E0) y2
  1110 1101 utf16! (ED) = U+D800–U+DFFF (surrogate halves=?) y3

  1111 0000 overlong +3 cont (F0) z0

  1111 0100 overlong +3 cont (F4) o0
  1111 0101  >U+10FFFF    (F5)    o1
  1111 0110  >U+10FFFF    (F6)    o2
  1111 0111  >U+10FFFF    (F7)    o3

  1111 1000 (5 byte utf8) (F8)      x0
  1111 1001      "        (F9)      x1
  1111 1010      "        (FA)      x2
  1111 1011      "        (FB)      x3
  1111 1100 (6 byte utf8) (FC)      x4
  1111 1101      "        (FD)      x5
  1111 1110 (7 byte utf8) (FE)      x6 
  1111 1111 (8 byte utf8) (FF) ???  x7

  FF - if in flash memory - unwritten1


  // SURROGATES (Unicode 2.0 smileys!)

  Code point area	Name
  U+D800 – U+DBFF	High Surrogates
  U+DC00 – U+DFFF	Low Surrogates

  A high surrogate code point followed by low surrogate code point is mapped to a supplementary code point. For example,

  U+ D80 0 +  DC 00 = U+01  0 0  00
  U+ D80 0 +  DF FF = U+01  0 3  FF

  U+ DBF F +  DC 00 = U+10  F C  00
  U+ DBF F +  DF FF = U+10  F F  FF

------------
not included:

¢£¤¥
¡¿¦¶
§©®
»«
¼½¾
±×÷·¸¬¯´¨ª°¹º²³µ

ÀÁÂÃ
ÇÈÉÏéïÊËÌÍÎ
ÐÑ
ÒÓÔÕ
ÙÚÛ
Ý

àáâã
çèêëìíî
ðñ
òóôõ
ùúû
ý
ÿ

              
              latin-1

<NBSP>¡¢£¤¥¦§¨©ª«¬<SHY>®¯
°±²³´µ¶·¸¹º»¼½¾¿
ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏ
ÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞß
àáâãäåæçèéêëìíîï
ðñòóôõö÷øùúûüýþÿ

*/

