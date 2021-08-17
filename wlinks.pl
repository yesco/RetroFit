$_= join('', <>);

while (m/\e\]:A:\{(.*?)\}\e\\/g) {
    # TODO: fill in and then extract
    $text= 'LINKTEXT';
    print "$1 $text\n";
}
