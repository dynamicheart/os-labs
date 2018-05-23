#!/usr/bin/perl -w

for(my $i = 0; $i < 256; $i++){
	if(!($i == 8 || ($i >= 10 && $i <= 14) || $i == 17)){
		print "TRAPHANDLER_NOEC($i)\n";
	} else {
		print "TRAPHANDLER($i)\n"
	}
}

print "\n# vector table\n";
print ".data\n";
print ".globl vectors\n";
print "vectors:\n";
for(my $i = 0; $i < 256; $i++){
	print "  .long vector$i\n";
}
