#!/usr/bin/perl

use CGI qw/:standard/;
use CGI::Carp qw(fatalsToBrowser warningsToBrowser);

print header;

print param("command");

open (PIPE, ">> ../../TestCode/Control/IPC_FIFO_RX");
print PIPE param("command") . "\n";
close PIPE;
