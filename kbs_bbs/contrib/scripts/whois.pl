use strict;
use FileHandle;
my $whoisserver = "whois.edu.cn";

#my $iplist="/etc/net-setting/edu";
#my $iplist="edu";

my $iplistfile = new FileHandle("@ARGV[0]");

sub ip_to_num {
    return undef if $_[0] =~ /[^0-9.]/;
    my @a = split /\./, $_[0];
    my $n = pop @a;
    return undef unless $n < ( 256**( 4 - @a ) );
    for ( 0 .. $#a ) {
        return undef unless $a[$_] < 256;
        $n += ( $a[$_] << ( 8 * ( 3 - $_ ) ) );
    }
    $n;
}

sub num_to_dotted_quad {
    my $num = shift;
    my @a   = ();
    while ($num) {
        unshift @a, sprintf( "%03d", $num & 0xFF );
        $num >>= 8;
    }
    join '.', @a;
}

sub dowhois {
    my ($ip) = @_;
    my $whois;
    my $cmd  = "whois " . num_to_dotted_quad($ip) . "@" . $whoisserver . "|";
    my $end  = 0;
    my $desc = "";
    open( $whois, $cmd );
    while ( my $line = <$whois> ) {
        if ( $line =~ /^inetnum:[^\d]+([\d\w\.]+)([^\d])+([\d\w\.]+)/g ) {
            my $sp  = $2;
            my $num = $3;
            if ( $num =~ /([\d]+\.[\d]+\.[\d]+\.[\d]+)/g ) {
                if ( $sp ge "/" ) {
                    $end = $ip + ip_to_num($num);

                    #                 print "find 1:" . $num . "\n";
                }
                else {
                    $end = ip_to_num($num);

                    #                 print "find 2:" . $num . "\n";
                }
            }
            else {
                $end = $ip + ( 1 << ( 32 - $num ) ) - 1;

                #                 print "find 3:" . $num . "\n";
            }
        }
        elsif ( ( $desc eq "" ) && ( $line =~ /^descr:[ \t]+(.+)/g ) ) {
            $desc = $1;
        }
        elsif ( $line =~ /^c-orgname:[ \t]+(.+)/g ) {
            $desc = $1;
        }
    }
    close $whois;

    #    print $ip . "\n" . $end . "\n";
    return ( $end, $desc );
}

sub search {
    my ( $from, $len ) = @_;

    my $curr = ip_to_num($from);
    my $allend = $curr + ( 1 << ( 32 - $len ) ) - 1;
    while ( $curr <= $allend ) {
        my ( $end, $desc ) = dowhois($curr);
        if ( $end != 0 ) {
            print num_to_dotted_quad($curr) . " "
              . num_to_dotted_quad($end) . " ["
              . $desc . "]\n";
            $curr = $end + 1;
        }
        else {

            #find next C class
            $curr += 256;
            print num_to_dotted_quad($curr) . " "
              . num_to_dotted_quad( $curr + 256 )
              . "cant find\n";
        }
    }
}

while ( my $line = <$iplistfile> ) {
    if ( $line =~ /([\d\w\.]+)\/(\d+)/g ) {
        search( $1, $2 );
    }
}
close($iplistfile);

