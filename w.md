# ./w - minimal webbrowser

# UI

## Top-line

Shows a spinning globe, if a page is loading, in the upper left corner. The rest of the line shows the current documents URL or TITLE.

## Content area

Shows rendered data, mostly HTML, but couild be output from any program.

## Mode-line

Like emacs, there is a mode line, below it shows messages as feedback. These may be error messages in red, or just normal white messages.

This may also be used as input buffer. That will clear any message.

# Interaction

<tt>.w</tt> borrows from both <tt>less</tt>, <tt>emacs</tt>, <tt>elinks</tt>, and <tt>w3m</tt>. It is mostly inteded to be a keyboard navigable browser, as it is run from the terminal.

## Navigation

<pre>
--- ./w
?                - help

C-c              - exit ./w
C-z              - suspend ./w (fg cont)

! cmd            - run command in shell
| cmd            - pipe webpage to cmd
" '              - quote page
(in cmd %f=file %u=url %l=%linksfile)

(: eval command   - hmmm?)
(; ???                   )

--- content
C-p, arrow up    - scroll up
C-n, arrow down  - scroll down
C-v, SPC         - next page
M-v, DEL/BS      - previous page
< (M-> and ',')  - beginning
> (M-< and '.')  - end
C-r              - reload
C-g              - goto line num

a..z RET         - new tab, open link
A..Z RET         - background open link
abc RET          - search links!

--- history
C-f, arrow right - history forward
C-b, arrow left  - history backward

--- tabs/reading flow
C-t              - open new (empty) tab

C-l, C-u         - open/edit/goto URL
C-g              - goto line, tab, 
C-o              - open file?

C-w              - close current, pop
C-k              - kill current, forward
C-j              - junk current, backs

C-y              - undo close

0-9              - goto user tab #

--- searching
C-s, /           - search forward
&                - narrowing search

--- bookmarks
C-d, *           - add bookmark, * it
#hashtag SPC/RET - add hashword/tag
#0 - #9 SPC/RET  - assign as tab #
$foo bar fie RET - store string for bookmark

=keyword ...     - search keyword?

@                - mail current page?
%                - scissor/cut-n-(paste)
() [] {}         - start end?

_
+ -

C-a beginning (tab 0, start tab)
C-e end (last tab)

C-q quit
C-x exit extra

C-l clear urL
C-u Url undo
</pre>

# Identifiers

<pre>
*               - star
#cool           - hashtab
#4711           - line in history
#-3             - line from current
#+3             - tab 3 (forward)

#2021-07-15     - match this tag date
#1629202308     - tag time in s
#1.629.202.308  - tag time in s
#1 629 202 308  - tag time in s
#Ae73Qrs        - timestamp s

</pre>

# History

History is about navigation in the <b>past</b>, or the <b>future</b>!

When <tt>./w</tt> starts it is at position 0, showing the last webpage you viewed last time, from the history. Using the <key>left</key> arrow, you can step backwards, and <key>right</key> forward, to the current point again. These are showned as <em>negative tabs</em>.

Any new tabs, or links you open, are opened in the background, and becomes available by moving forward, by using the <key>right</key>. Clicking on a link opens the a new tab and goes to it.

## ./whistory file

Whenever a page is entered through command line, or navigated to by clicking on a link, the <tt>./wdownload</tt> script runs, if not already cached and formatted, it will do that. It gives interactive status messages (from the background) in the mode-line. <tt>./wdownload</tt> also appends a line to <tt>.whistory</tt> file. It contains an ISO-timestamp down to the seconds, including the time-zone, as well as the URL. Here is an example:

<pre>
# TIMESTAMP               #=W URL CACHE-FILE

2021-08-16T04:00:06+07:00 #=W yesco.org .w/Cache/yesco.org.ansi
2021-08-16T04:00:17+07:00 #=W http://yesco.org .w/Cache/http:%2f%2fyesco.org.ansi
</pre>

# Bookmarks

