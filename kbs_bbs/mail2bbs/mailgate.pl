#!/usr/bin/perl
sub readmail {
    @MAIL = <STDIN>;
    &decode_mail;
    &parse_header;
}

sub decode_mail {
    foreach $n ( 0 .. $#MAIL ) {
        $line = $MAIL[$n];
        if ( substr( $line, 0, 6 ) eq "begin " ) {
            if ( $line =~ /^begin \d+ \w/ ) {
                &uudecode($n);
                return;
            }
        }
    }
}

sub uudecode {
    local ($begin) = @_;

    $line = $MAIL[ $begin + 1 ];
    $ch   = substr( $line, 0, 1 );
    $len  = int( ( ord($ch) - ord(' ') + 2 ) / 3 ) * 4;
    return if ( $len != ( length($line) - 2 ) );
    $tmpfile = "/tmp/mail2newsdecode.$$";
    open( FN, "| /bin/uudecode" );
    print FN "begin 644 $tmpfile\n";
    foreach $n ( $begin + 1 .. $#MAIL ) {
        print FN $MAIL[$n];
    }
    close(FN);
    $#MAIL = $begin - 1;
    open( FN, $tmpfile );
    while (<FN>) {
        $MAIL[@MAIL] = $_;
    }
    close(FN);
    unlink($tmpfile);
}

sub parse_header {
    local ($index);
    for ( $index = 0 ; $index <= $#MAIL ; $index++ ) {
        $_ = $MAIL[$index];
        last if ( $_ eq "\n" );
        chop;
        if (/^(?:(\S+): ([^;]*))?;?\s*(?:(\S+)="(.*)")?(?:(\S+)=(.*))?$/) {
            $hhh = $Header{$1};
            if ( $1 eq "Received" && $hhh ) {
                $tmp = "$2";
                if ( $tmp =~ /localhost/ ) {
                    $Header{'Received'} = $hhh;
                }
                else {
                    $Header{'Received'} = $tmp;
                }
            }
#            elsif ( $1 eq "Content-Type" ) {
#               if ( $MAIL[$index+1] =~ /^\s*boundary="(.+)"$/ ) { 
#                       $Header{"boundary"} = $1;
#               }
#           } 
            else {
                $Header{$1} = $2;
                $Header{$3} = $4;
                $Header{$5} = $6;
            }
        }
    }
    $from = $Header{'from'};
    if ( $from =~ /^(.*) <(\S+)>/ ) {
        $from = "$2 (${1})";
    }
    elsif ( $from =~ /^<(\S+)>/ ) {
        $from = "$1";
    }
    if ( index( $from, "@" ) < 0 ) {
        if ( $from =~ /^(\S*) \((.*)\)$/ ) {
            $from = "$1\@bbs.feeling.smth.org ($2)";
        }
        else {
            $from = "${from}\@bbs.feeling.smth.org";
        }
    }
    $from =~ s/\'/\"/g;
    $subject      = $Header{'subject'};
    $subject      = '(none)' unless $subject;
    $lastline     = $index + 100;
    $lastline     = ( $#MAIL < ${lastline} ) ? $#MAIL : $lastline;
    $subjectfound = 0;
    $fromfound    = 0;
    $passwdfound  = 0;
    $bbsfound     = 0;

    for ( $i = $index ; $i <= $lastline ; $i++ ) {
        $_ = $MAIL[$i];
        chop;
        next if ( $_ eq "" );
        if ( substr( $_, 0, 9 ) eq "subject: " ) {
            if ( !$subjectfound ) {
                $subject = substr( $_, 9 );
                $subjectfound = 1;
            }
        }
        elsif (/^[\s]*\#(\w+)[\s:]+(.*)$/) {
            $1 =~ tr/A-Z/a-z/;
            $BBSHeader{$1} = $2;
            $bbsfound = 1;
        }
        else {
            last;
        }
    }
    $name    = $BBSHeader{'name'};
    $title   = $BBSHeader{'title'};
    $title   = $BBSHeader{'subject'} unless $title;
    $passwd  = $BBSHeader{'password'};
    $passwd  = $BBSHeader{'passwd'} unless $passwd;
    $subject = $title if ($title);
    if ( $name && $passwd ) {
        die "$nickname $!\n";
    }
    $Header{'from'}    = $from;
    $Header{'subject'} = $subject;
    $MAILBODY          = $i;
}
1;
