$_= join('', <>);

while (m/\e\]:A:\{(.*?)\}\e\\/g) {
    print "$1\n";
}
