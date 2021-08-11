while (<>) {
    if (m/^([^\r\n]+)\r/) {
        print "$1\n";
    }
    while (m/\]:([^\r\n]+?)\\/gms) {
        print "  $1\n";
    }
}
