$_ = join('', <>);

s:<!--.*?-->::ismg;

s:<style[\s\S]*?</style>:<style>STYLE</style>:ismg;

s:<script([^>]*>)[\s\S]*?</script>:<script$1--script--</script>:ismg;

s:(<\s*\w+.*?>):tag($1):eismg;

s:(<\s*\w+.*?>)(.*?)(<\s*\/?\w+\s*/?\s*>):tag($1).$2.$3:eismg;

#s:(<script[^>]*>)[\s\S]*?</script>:$1SCRIPT</script>:ismg;

s:(\s*\n)+:\n:ismg;

s:(\s*\n)+:\n:ismg;
s:\t+: :ismg;
s: +: :ismg;
s:\n\s+:\n:ismg;

print $_;

sub tag {
    my ($s) = @_;
    $s =~ s:\s+: :smg;
    return $s if $s =~ m:<a:i;
    return $s if $s =~ m:<img:i;
    #return $s;
    $s =~ s:"([^"]|\\")*"::smg;
    $s =~ s:'([^']|\\')*'::smg;
    #return "FOO$s";
    return $s;
}
