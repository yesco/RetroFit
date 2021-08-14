# format history/log entries for huamns
#
# assumes to read them in reverse order on stdin
#
# will display:
# 2021-08-14
# - 14:36:27 #=W ...
my $lastdate;
while(<>) {
    if (/^(\d+-\d+-\d+)T(\d+:\d+:\d+)\S+ (.*)/) {
        my ($date, $time, $log) = ($1, $2, $3);
        unless ($date eq $lastdate) {
            print "$date\n";
            $lastdate= $date;
        }
        print "- $time $log\n";
    }
}
