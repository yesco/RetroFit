# jsk note: can't run on Termux as Term::Size does NOT install OK.


#  	PerlMonks  
# Pikture - yet another ascii art drawing program
# by X-3mE	Log inCreate a new user The Monastery Gates Seekers of Perl Wisdom Meditations Cool Uses For Perl Obfuscation Tutorials Poetry Reviews Perl News Donate Recent Threads Newest Nodes Super Search PerlMonks Discussion What's New
# on Jun 10, 2005 at 18:48 UTC ( #465641=sourcecode: print w/replies, xml )	Need Help??
# Category:	Fun Stuff
# Author/Contact Info	X-3mE'89 - website: http://exxtreme.altervista.org - mailto: exxtreme aT altervista DoT org
# Description:	It's been a while since my last post. Anyway, here comes a nice ascii-art drawing program, but it *works only under xterm* because it uses xterm's DEC private mode attribs. It uses Term:: and escape sequences for the "gui".
# Enjoy and let me know if you like it :)

#!/usr/bin/perl
#
# -*- perl -*- (this line is for emacs)
#
# Pikture
# Xterm/Ascii art drawing program
# Written by X-3mE'89 - http://exxtreme.altervista.org
# Released under GNU GPL terms
#
# Big thanks to Alessandro Rubini, author of GPM,
# that inspired this code
#

#use strict;
#use warnings;

use Term::ReadKey;
#use Term::Size;
use Term::ReadLine;

#
# Escape sequences for Xterm mouse handling
# (DEC private mode set/reset)
#

my $Dec_Pvt_Mode_Set = "h";         # DEC private mode set
my $Dec_Pvt_Mode_Reset = "l";       # DEC private mode unset

my $VT200_Mouse = "1000";           # DEC pvt mode for VT200 mouse
my $VT200_HighLight_Mouse = "1001"; # DEC pvt mode for VT200 hilite mo

my $Btn_Event_Mouse = "1002";       # DEC pvt mode: detect button even+ts
my $Any_Event_Mouse = "1003";       # DEC pvt mode: detect any mouse e+vent

my %MouseState;
$MouseState{'x'}      = 0;          # mouse X
$MouseState{'y'}      = 0;          # mouse Y
$MouseState{'mod'}    = 0;          # mode (pressed/released)
$MouseState{'btn'}    = 0;          # button
$MouseState{'wheel'}  = 0;          # was wheel moved?
$MouseState{'moving'} = 0;          # detect mouse motion

my $PenChar = '+';

my $Columns = 0;
my $Lines = 0;
my @Screen = undef;

# Raw mode. I chose to put this in a variable for debugging.
my $Read_Mode = 4;

#
# DecSet
# Accepts a list of modes and prints escape sequences that set the
# associate DEC private mode
#

sub DecSet
{
    my $what = shift;
    print "\e[?".$what.$Dec_Pvt_Mode_Set;
}

#
# DecUnset
# Accepts a list of modes and prints escape sequences that reset the
# associate DEC private mode
#

sub DecUnset
{
    my $what = shift;
    print "\e[?".$what.$Dec_Pvt_Mode_Reset;
}

sub InitScr
{
    print "\ec";
    ReadMode $Read_Mode;
    DecSet $VT200_Mouse;
    #DecSet $VT200_HighLight_Mouse;
    DecSet $Btn_Event_Mouse;
    DecSet $Any_Event_Mouse;
    # jsk:
    #($Columns, $Lines) = Term::Size::chars *STDOUT{IO};
    ($Columns, $Lines) = ($ENV{"COLUMNS"}, $ENV{"LINES"});
}

sub EndScr
{
    print "\n";
    DecUnset $VT200_Mouse;
    #DecUnset $VT200_HighLight_Mouse;
    DecUnset $Btn_Event_Mouse;
    DecUnset $Any_Event_Mouse;
    print "\ec";
    ReadMode 0;
}

sub BeginMouse
{
    DecSet $VT200_Mouse;
    DecSet $Btn_Event_Mouse;
    DecSet $Any_Event_Mouse;
}

sub EndMouse
{
    DecUnset $VT200_Mouse;
    DecUnset $Btn_Event_Mouse;
    DecUnset $Any_Event_Mouse;
}

#
# Paint
# Paints on the screen.
#

sub Paint
{
    if($MouseState{'btn'} == 0)
    {
        print "\e[".$MouseState{'y'}.";".$MouseState{'x'}."H".
            $PenChar;
        print "\e[0;0H";
        $Screen[$MouseState{'y'}][$MouseState{'x'}-1] = $PenChar;
    }
}

#
# HandleChars
# Handle input chars != mouse events
#

sub HandleChars
{
    my $char = chr shift;
    if(($char eq 'p') or ($char eq 'P'))
    {
        ReadMode 0;
        EndMouse;
        print "\e[";
        print $Lines-2;
        print ";0HPen char: ";
        $PenChar = ReadKey;
        ReadMode $Read_Mode;
        BeginMouse;
        print "\e[".($Lines-2).";0H";
        for(my $i = 0; $i < $Columns; $i++)
        {
            print " ";
        }
        print "\e[".($Lines-2).";0H";
    }
    if(($char eq 'c') or ($char eq 'C'))
    {
        InitScr;
        my ($i,$j);
        for($i = 0; $i < $Lines; $i++)
        {
            for($j = 0; $j < $Columns; $j++)
            {
                $Screen[$i][$j] = " ";
            }
        }
    }
    if(($char eq 'r') or ($char eq 'R'))
    {
        InitScr;
        my ($i, $j);
        for($i = 0; $i < $Lines; $i++)
        {
            for($j = 0; $j < $Columns; $j++)
            {
                print $Screen[$i][$j];
            }
            my $real = $i + 1;
            print "\e[$real;0H";
        }
    }
    if(($char eq 's') or ($char eq 'S'))
    {
        ReadMode 0;
        EndMouse;
        print "\e[";
        print $Lines-2;
        print ";0HFile name: ";
        my $Fname = ReadLine;
        ReadMode $Read_Mode;
        BeginMouse;
        print "\e[".($Lines-2).";0H";
        for(my $i = 0; $i < $Columns; $i++)
        {
            print " ";
        }
        print "\e[";
        print $Lines-1;
        print "0H";

        open(FILE, ">$Fname") or die "Couldn't open $Fname: $!\n";

        my ($i, $j);
        for($i = 0; $i < $Lines; $i++)
        {
            for($j = 0; $j < $Columns; $j++)
            {
                print FILE $Screen[$i][$j];
            }
            print FILE "\n";
        }

        close FILE;
        ReadMode $Read_Mode;
        BeginMouse;
    }
}

#
# MouseEvents
# Handle mouse events, and put the result in %MouseEvents.
#

sub MouseEvents
{
    my $btn = ord(ReadKey) - 32;
    my $mod = $btn & 28;    # get other 3 bits  (mask = 00011100)
    $mod >>= 3;        # shift 3 bits right
    my $wheel = 0;
    my $moving = 0;

    if($mod != 0)
    {
        $btn &= ~28;    # remove modifier info
    }
    
    if($btn >= 64)          # ... then wheel has been rolled
    {
        $wheel = 1;
        $btn -= 64;
    }
    elsif($btn >= 32)       # ... then mouse has been moved
    {
        $moving = 1;
        $btn -= 32;
    }
    
    my $x = ord(ReadKey) - 32;
    my $y = ord(ReadKey) - 32;

    #
    # Fill %MouseState's fields
    #

    $MouseState{'x'} = $x;
    $MouseState{'y'} = $y;
    $MouseState{'mod'} = $mod;
    $MouseState{'btn'} = $btn;
    $MouseState{'wheel'} = $wheel;
    $MouseState{'moving'} = $moving;

    printf "\e[0;0H[P]en - [C]lear - [R]efresh - [Q]uit\n".
        "\e[".($Lines-1).";0HBtn=%3d; X=%3d; Y=%3d; Mod=%3d; ".
        "Wheel=%3d; Moving=%3d",
        $btn, $x, $y, $mod, $wheel, $moving;

    Paint;
}

################## ###  ##   #
#
# Main loop
#
################## ###  ##   #

#
# Clear screen and set read mode to $Read_Mode
#

InitScr;

my ($i,$j);
for($i = 0; $i < $Lines; $i++)
{
    for($j = 0; $j < $Columns; $j++)
    {
        $Screen[$i][$j] = " ";
    }
}

my $key = "";

$| = 1; # autoflush

while(($key = ReadKey) ne 'q')
{
    my $code = ord $key;
    if($code == 0x1b)    # got an escape seq ("\e")
    {
        # is it 0x5b ([) ?
        $key = ReadKey;
        $code = ord($key);
        next if($code != 0x5b);    # 0x5b = '['
        my $what = ord(ReadKey);
        if($what == 0x4d)    # 0x4d = 'M'
        {
            MouseEvents;
        }
    }
    else
    {
        HandleChars($code)
    }
}

EndScr;
ReadMode 0;

# Comment on Pikture - yet another ascii art drawing programDownload Code
# Replies are listed 'Best First'.
# Re: Pikture - yet another ascii art drawing program
# by chanio on Jun 10, 2005 at 20:19 UTC
# Thank you, I'll give it a try!
# For the rest that doesn't know about the best multiplatform ASCII art editor (written in JAVA) just visit the JAVE website.

# Only if you want to have some fun. I don't think that everybody might enjoy doing some drawings with monospaced letters & numbers. Or doing an ASCIIart film, or printing an animated GIF with ASCIIart...

# It also has some JAVE pms at CPAN.

# .{\('v')/}   C H E E R   U P !
#  _`(___)' ___a_l_b_e_r_t_o_________
# Wherever I lay my KNOPPIX disk, a new FREE LINUX nation could be established.
# [reply]
# Re: Pikture - yet another ascii art drawing program
# by dReKurCe on Jun 13, 2005 at 21:10 UTC
# Quite a functional piece of code...I like it!!
