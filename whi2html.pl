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
        # load from .TITLE
        if (!$title) {
            my $ft=$ansi;
            $ft =~ s/.ANSI/.TITLE/;
            if (open FIL, $ft) {
                $title= <FIL>;
                close FIL;
            }
        }
        if (!$title) {
            $title = $url;
            $title =~ s/%(..)/chr($1)/eg;
            #$title =~ s:\.(org|com|net|biz)::;
            $title =~ s|^https?:/+||;
            $title =~ s|^www.||;
            $title =~ s|[./]?html?||;
            $title =~ s:\b(index|cached|images|image|picture|icon|cache|cdn|news|static|dynamic|main|desktop|large|small|medium|en|US|de|archives?)\b: :ig;
            $title =~ s|fbclid=.*||;
            $title =~ s|/?#?$/?||;
            $title =~ s|[0-9a-f_-]{5,}| |ig;
            $title =~ s|/(.*?)/([^/]{5,})$|... $2|;

            $title =~ s|-+| |g;
            $title =~ s| +| |g;
            $title =~ s|\s*($1)\s*|$1|;
            $title =~ s|\?q=([^&]+)| Seach: $1|;
            $title =~ s|\?(.*)||;
            $title =~ s|.?blogspot.com| Blog |;
            $title =~ s|#$||g;
            $title =~ s|/+$||g;
        }
        $title =~ s/[ \s-_\.]{2,}/ /g;
        $title =~ s|^[\.\s]*(.*)[\.\s#?/]*$|$1|;

        unless ($url =~ /(https:|http:|file:)/) {
            $url= "http://" . $url;
        }
        print "$hhmm <a href=\"$url\"> $title</a>"; # <br> if not pre
        print ($pre? "\n" : "<br>\n");

    } else {
        print  STDERR "%%: $_";
    }
}

print "</pre>\n";
