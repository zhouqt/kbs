#!/usr/local/bin/perl
sub getremoteinfo {
  require "/home/bbs/bin/rfc931.pl";
  ($there,$here)=(getpeername(STDOUT),getsockname(STDOUT));
  ($sockaddr)= 'S n a4 x8';
  ($family,$thisport,$thisaddr)=unpack($sockaddr,$here);
  ($family,$thatport,$thataddr)=unpack($sockaddr,$there);
  (@localaddr)=unpack('C4',$thisaddr);
  (@remoteaddr)=unpack('C4',$thataddr);
  ($hostname)=gethostbyaddr($thisaddr,2);
  ($remotehostname)=gethostbyaddr($thataddr,2);
  $username=&rfc931_name($there,$here);

  if (! $username) {
        $username="unknown";
  }
  if (index($hostname,".")<0 && index($remotehostname,".")>=0) {
        $hostname = "${hostname}.cs.ccu.edu.tw";
  }
  return ($username,$hostname);
}

sub readmail {
	($newsgroups) = @_;
	$bbscheck = "/usr/spool/news/bbsaccess/bbscheck";
	@MAIL = <STDIN>;
	&decode_mail;
	&parse_header;
}

sub ndreadmail {
	($newsgroups) = @_;
	$bbscheck = "/usr/spool/news/bbsaccess/bbscheck";
	@MAIL = <STDIN>;
	&parse_header;
}

sub decode_mail
{
    foreach $n (0 .. $#MAIL) {
        $line = $MAIL[ $n ];
        if( substr( $line, 0, 6 ) eq "begin " ) {
            if( $line =~ /^begin \d+ \w/ ) {
                &uudecode( $n );
                return;
            }
        }
    }
}
sub uudecode
{
    local       ($begin) = @_;

    $line = $MAIL[ $begin + 1 ];
    $ch = substr( $line, 0 , 1);
    $len=int((ord($ch) - ord(' ')+2)/3) * 4 ;
#    print "ch $ch len $len length ",length($line),"\n";
    return if ($len != (length($line)-2));
#    if( substr( $line, 0, 1 ) ne "M" ) {
#        return;
#    }
    $tmpfile = "/tmp/mail2newsdecode.$$";
    open( FN, "| /bin/uudecode" );
    print FN "begin 644 $tmpfile\n";
    foreach $n ($begin + 1 .. $#MAIL) {
        print FN $MAIL[ $n ];
    }
    close( FN );
    $#MAIL = $begin - 1;
    open( FN, $tmpfile );
    while( <FN> ) {
        $MAIL[ @MAIL ] = $_;
    }
    close( FN );
    unlink( $tmpfile );
}


sub parse_header {
  local($index);
  for ($index=0; $index <= $#MAIL; $index++) {
        $_ = $MAIL[ $index ];
	last if ($_ eq "\n");
	chop;
	if (/^(\S+): (.*)$/) {
		$hhh = $Header{$1};
		if ( $1 eq "Received" && $hhh ) {
		  $Header{'Received'}="$hhh\t$2";
		} else {
		  $Header{$1} = $2;
		}
	}
  }
  $from = $Header{'from'};
  if ( $from =~ /^(.*) <(\S+)>/ ) {
	$from = "$2 (${1})";
  } elsif ( $from =~ /^<(\S+)>/ ) {
	$from = "$1";
  }
  if( index( $from, "@" ) < 0 ) {
    if( $from =~ /^(\S*) \((.*)\)$/ ) {
	$from = "$1@cs.ccu.edu.tw ($2)";
    } else {
	$from = "${from}@cs.ccu.edu.tw";
    }
  }
  $from =~ s/\'/\"/g;
  $subject = $Header{'subject'};
  $subject = '(none)' unless $subject;
  $lastline = $index + 100; 
  $lastline = ($#MAIL < ${lastline})? $#MAIL : $lastline;
  $subjectfound = 0;
  $fromfound = 0;
  $passwdfound = 0;
  $bbsfound = 0;
  for ($i=$index  ; $i <= $lastline; $i++) {
	$_ = $MAIL[ $i ];
	chop;
	next if ($_ eq "");
	if (substr($_,0,9) eq "subject: ") {
	    if (! $subjectfound ) {
		 $subject = substr($_,9);
		 $subjectfound = 1;
	    }
	} elsif (/^[\s]*\#(\w+)[\s:]+(.*)$/) {
	    $1 =~ tr/A-Z/a-z/;
	    $BBSHeader{$1} = $2;	
	    $bbsfound = 1;
        } else {
	    last;
	}
  }
  $name =  $BBSHeader{'name'};
  $title = $BBSHeader{'title'};
  $title = $BBSHeader{'subject'} unless $title;
  $passwd = $BBSHeader{'password'};
  $passwd = $BBSHeader{'passwd'} unless $passwd;
  $passwd = $BBSHeader{'passward'} unless $passwd;
  $subject = $title if ($title);
  if ($name && $passwd) {
	open(CHECK,"$bbscheck $name $passwd |");	
	chop($nickname = <CHECK>);
	close(CHECK);
        if ($? == 0) {
		if ($newsgroups) {
			@groups = split(/\,/,$newsgroups);
			foreach $group (@groups) {
				if (substr($group,0,9) ne 'csie.bbs.' &&
                                    substr($group,0,7) ne 'tw.bbs.'){
					die "trying to post by BBS user $name, \nbut BBS user can't post to one of the following newsgroups:\n$newsgroups\n";
				}
			}
			$realfrom = $from unless $realfrom;
			$from="${name}.bbs@firebird.cs.ccu.edu.tw (${nickname})";
		} else {
			$realfrom = $from unless $realfrom;
			$from="${name}.bbs@firebird.cs.ccu.edu.tw (${nickname})";
		}
	} else {
		die "$nickname $!\n";
	}
  }
  $Header{'from'} = $from;
  $Header{'subject'} = $subject;
  $MAILBODY = $i;
}
1;
