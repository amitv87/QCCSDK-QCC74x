#!/usr/bin/env perl

print "/* Automatically generated, please do not modify! */
#ifndef _CHECK_MACSW_CFG_
#define _CHECK_MACSW_CFG_\n";

while(<STDIN>)
{
$_ =~ /-D([^=]*)=?([^=]*)\n/;
if($2 eq "")
{
print "#ifndef $1
 #error \"Missing $1 Param.\"
#endif\n";
}
else {
print "#ifdef $1
 #if $1 != $2
  #error \"$1 not compatible\"
 #endif
#else
 #error \"Missing $1 Param.\"
#endif\n";
}

}
print "#endif";
