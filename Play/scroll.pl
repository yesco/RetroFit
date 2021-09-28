system "rm scroll.log";

for $i (1..300) {
    print "\e[0,0H";
    system("./wdisplay .w/Cache/runeberg.org.ANSI $i ".($ENV{'LINES'}-10)." ".$ENV{'COLUMNS'});
}
