#

while ($line = <> )
{
   $line =~ s/\r\n/\n/g;
   print $line;
}
