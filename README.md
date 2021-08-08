# RetroFit

A retro-themed web-browser for the terrminal with command line.

# Purpose

An experiment in minimalistic textual web experience meant to be easily ported to retro-homoecomputer architectures such as 6502, z80.

# Features

- Parses and displays simlified HTML
- Using ANSI colors for retro-style display
- CSS is ignored
- Javascript is ignored
- No cookies, GDR, popups etc
- Aims to minimize latency as in TTFC (Time To First Char)

# Origin

It started as a feature of the ORIC ATMOS WebBasic Simulator, running in the browser.

# Versions

- retro.js is an extract of that code, incomplete, and unfinished. This can be used as a browser inside the browser. LOL
- w.c is a smaller plain C-code implementation.

# How to use

Well, it's not in any place where normal sites can be used, or displayed in pleasing ways, but it's a funny interesting concept.

Files:
- *./w<* - user script to call, will compile if w.c changed
- *w.c* - the actual code - the workhorse.</li>
- *entities.h* - list of HTML entities stored in one long string, genereated from entities.json using entities.js and then sow manual editing (already done - only needs to be done one time)

Test files:
- *./w test.html* - general testfile and examples, and bug-tracker!
- *./w google.com* - rendered like crap; Google uses a lot of &#nnnn; entities.
- *w* - in this start/compile script, there are many URLs

   w.c - the actual code to render

# Wants

- a modular streamed browser
- commands can be external independent scripts
- easily extensible (Unix-style/CGI-style)
- plain C to be portable to even smaller 8-bit systems

# Possible TODO:

- [ ] stream download HTML
- [ ] stream parse HTML
- [ ] serial out
- [ ] ansi display HTML serially
- [ ] go link by number
- [ ] go link by
- [ ] page down
- [ ] line down
- [ ] beginning
- [ ] page up
- [ ] 
- [ ] keep history (append)
- [ ] tabs/several buffers
- [ ] bookmarks?
- [ ] cookies?
- [ ] news (check bookmarks, sort, order)

# Graphics
- Look into caca-library (https://github.com/cacalabs/libcaca)



