$_ = join('', <>);

s:<!--.*?-->::ismg;

s:<style[\s\S]*?</style>:<style>STYLE</style>:ismg;

s:<script([^>]*>)[\s\S]*?</script>:<script$1--script--</script>:ismg;

# simplify tags, remove most attribute values
s:(<\s*\w+.*?>):tag($1):eismg;

s:(<\s*\w+.*?>)(.*?)(<\s*\/?\w+\s*/?\s*>):tag($1).$2.$3:eismg;

#s:(<script[^>]*>)[\s\S]*?</script>:$1SCRIPT</script>:ismg;

# remove superfluos whitespace
s:(\s*\n)+:\n:ismg;
s:\t+: :ismg;
s: +: :ismg;
s:\n\s+:\n:ismg; # this removes indent?

print $_;

sub tag {
    my ($s) = @_;

    # DEBUG: <foo ...>FOO:
    #$s =~ s/^<(\w+)(.*)>$/<$1$2>$1:/g;

    # Don't modify <a> and <img>
    return $s if $s =~ m:<a:i;
    return $s if $s =~ m:<img:i;
    #return $s;

    # TODO: keep some attributes
    # remove attribute valuies (long)
    $s =~ s:"([^"]|\\"){15,}"::smg;
    $s =~ s:'([^']|\\'){15,}'::smg;

    return $s;
}
