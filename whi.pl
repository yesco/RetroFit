# format history/log entries for huamn#
# Usage:
#   ./whistory | perl ./whi.pl 
#   ./whistory | perl ./whi.pl "#=W"
#
# Read and format lines from stdin.
#
# Optional ARGUMENT is used to filter
# and is removed from the string.
#
# It assumes the data comes in reverse order on stdin
# (output of ./whistory  or ./wlog)
#
# will display:
#   2021-08-14
#   - 14:36:27 ...
#   - 14:36:27 ...
my $m= $ARGV[0];
my $lastdate;
while(<STDIN>) {
    if ($m) {
        next unless s/$m\s+//;
    }
    
    if (/^(\d+-\d+-\d+)T(\d+:\d+:\d+)\S+ (.*)/) {
        my ($date, $time, $log) = ($1, $2, $3);
        unless ($date eq $lastdate) {
            print "$date\n";
            $lastdate= $date;
        }
        print "- $time $log\n";
    }
}
