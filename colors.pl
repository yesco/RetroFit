#
# Using:
#   cat colors.txt | perl colors.pl > colors.h

# colors data from:
# - https://www.w3.org/TR/css-color-3/#color0

print 'const char COLORS[] = ""';
while (<>) {
    s/\s\d.*//g;
    s/\t//g;
    chop;
    print "\" $_\"\n";
}
print '"";';
