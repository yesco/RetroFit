// --- ansi terminal functionality to capture mouse clicks or drags!
//
// "\e[?1000h" # to enable Send Mouse X & Y on button press and release

// "\e[?9h" # to enable Send MIT Mouse Row & Column on Button Press

// https://invisible-island.net/xterm/ctlseqs/ctlseqs.html#h2-Mouse-Tracking

// echo -e "\033[6n"
// - https://stackoverflow.com/questions/8343250/how-can-i-get-position-of-cursor-in-terminal/8353312#8353312

// $ perl -e '$/ = "R";' -e 'print "\033[6n";my $x=<STDIN>;my($n, $m)=$x=~m/(\d+)\;(\d+)/;print "Current position: $m, $n\n";'

// --- terminal size
// - https://stackoverflow.com/questions/8343250/how-can-i-get-position-of-cursor-in-terminal/8353312#8353312
// echo -e "lines\ncols"|tput -S

// --- terminal resize event?
// https://docstore.mik.ua/orelly/unix3/upt/ch05_05.htm
// - https://unix.stackexchange.com/questions/16578/resizable-serial-console-window/283206
// - SIGWNCH
// - https://forums.justlinux.com/showthread.php?137431-ncurses-detect-when-the-terminal-has-been-resized
// - https://superuser.com/questions/390935/how-can-i-get-terminal-to-repaint-the-contents-when-i-resize-the-terminal

// --- TPUT commands
// - https://www.gnu.org/software/termutils/manual/termutils-2.0/html_chapter/tput_1.html


// --- ncurses

// --- nodejs ncurses
// - https://github.com/chjj/blessed

