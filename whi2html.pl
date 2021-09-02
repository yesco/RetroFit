$reverse = 1;
if ($reverse) {
    open HI, "tac .whistory|";
} else {
    open HI, ".whistory";
}

# use full width

$pre = 0;

print "<pre>\n" if $pre;

my $lastdate, $lasturl;
while (<HI>) {
    chop();
    if (/(\S+?)T((\d\d:\d\d):\d\d)\S+\s+(\S+)\s+(\S+)\s+(\S+)(|\s+(.*))/) {
        my ($date, $hhmmss, $hhmm, $type, $url, $ansi, $title) = ($1, $2, $3, $4, $5, $6);
        #print "date=$date\nhhmm=$hhmm\nhhmmss=$hhmmss\ntype=$type\nurl=$url\nansi=$ansi\ntitle=$title\n\n";

        unless ($date eq $lastdate) {
            print "<h2>$date</h2>\n";
            $lastdate = $date;
        }

        next if $url eq $lasturl;
        $lasturl= $url;

        # if no title use simplified url
        if ($url =~ m|duckduckgo\.com/html\?q=(.*)|i) {
            $title = "Search: $1";
        }
        if (!$title) {
            $title = $url;
            #$title =~ s:\.(org|com|net|biz)::;
            $title =~ s|^https?://||;
            $title =~ s|[./]?html?|...|;
            $title =~ s|index|...|g;
        }
        $title =~ s/\+/ /g;
        $title =~ s/%(..)/chr($1)/eg;

        print "$hhmm <a href=\"$url\"> $title</a>"; # <br> if not pre
        print ($pre? "\n" : "<br>\n");

    } else {
        print  STDERR "%%: $_";
    }
}

print "</pre>\n";
