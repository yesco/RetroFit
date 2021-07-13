// ansi terminal functionality to capture mouse clicks or drags!
//
// "\e[?1000h" # to enable Send Mouse X & Y on button press and release

// "\e[?9h" # to enable Send MIT Mouse Row & Column on Button Press

// https://invisible-island.net/xterm/ctlseqs/ctlseqs.html#h2-Mouse-Tracking

// echo -e "\033[6n"
// https://stackoverflow.com/questions/8343250/how-can-i-get-position-of-cursor-in-terminal/8353312#8353312

// $ perl -e '$/ = "R";' -e 'print "\033[6n";my $x=<STDIN>;my($n, $m)=$x=~m/(\d+)\;(\d+)/;print "Current position: $m, $n\n";'

