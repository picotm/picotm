#!/usr/bin/env perl
#
# picotm - A system-level transaction manager
# Copyright (c) 2017    Thomas Zimmermann <contact@tzimmermann.org>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

# Run this script in the repository's top-level directory to generate
# a new release.

# Perl language settings
use v5.22;
use autodie;
use strict;
use warnings;

# Modules
use Cwd;
use File::Copy qw/ copy move /;
use File::Spec;
use File::Temp qw/ tempdir tempfile /;
use Getopt::Long;
use IO::Handle;

my $opt_release_abi_updated = undef;
my $opt_package = 1;
my $release_type = undef;
my $opt_tag = 1;
my $opt_username = undef;

GetOptions('release-abi-updated!' => \$opt_release_abi_updated, # flag
           'package!'             => \$opt_package,  # flag
           'release-type=s'       => \$release_type, # string: 'major', 'minor',
                                                     #         'micro'
           'tag!'                 => \$opt_tag,      # flag
           'username=s'           => \$opt_username) # string: 'GPG user name'
    or die;

die('Set --release-abi-updated if you updated the ABI version')
    unless defined $opt_release_abi_updated;

die('Undefined release type') unless defined $release_type;

die("Invalid release type '$release_type'")
    unless grep(/^$release_type$/, ('major', 'minor', 'micro'));


#########################################################################
#                                                                       #
#   CWD Helpers                                                         #
#                                                                       #
#########################################################################

# Execute a function in a directory.
#
sub executeInDir
{
    my $execute = shift(@_);
    my $path = shift(@_);

    my $dir = getcwd();

    chdir($path);

    my @result = &$execute(@_);

    chdir($dir);

    return @result;
}

#########################################################################
#                                                                       #
#   File Helpers                                                        #
#                                                                       #
#########################################################################

# Opens a file for parsing.
#
sub parseFile
{
    my $parse = shift(@_);
    my $filename = shift(@_);

    open(my $fh, '<', $filename) or die;

    my @result = &$parse($fh, $filename, @_);

    close($fh) or die;

    return @result;
}

# Generates a new file.
#
sub generateFile
{
    my $generate = shift(@_);
    my $filename = shift(@_);

    open(my $fh, '>', $filename) or die;

    &$generate($fh, $filename, @_);

    close($fh) or die;
}

# Modifies a file according to a transformation routine. The change is
# applied atomically.
#
sub transformFile
{
    my $parse = sub {

        my $fh = shift(@_);
        my $filename = shift(@_);
        my $transform = shift(@_);

        my ($tempfh, $tempfilename) = tempfile();

        &$transform($fh, $filename, $tempfh, $tempfilename, @_);

        close($tempfh) or die;

        return ($tempfilename);
    };

    my $transform = shift(@_);
    my $filename = shift(@_);

    # We first parsing the file into the transformation function. This
    # will fill a temporary file.
    my ($tempfilename) = parseFile($parse, $filename, $transform, @_);

    # Afterwards, we move the temporary file into the old file's position.
    say "Moving $tempfilename to $filename";
    move($tempfilename, $filename);
}


#########################################################################
#                                                                       #
#   configure.ac                                                        #
#                                                                       #
#########################################################################

# Parse configure.ac for version number.
#
sub parseConfigureAc {

    my $parse = sub {

        my ($fh, $filename) = @_;

        my $package = undef;
        my $major = undef;
        my $minor = undef;
        my $micro = undef;

        while (<$fh>) {
            if ( $_ =~ /^AC_INIT\(\[(\w+((-|_)\w+)?)\],\s*\[(\d+)\.(\d+)\.(\d+)\]/ ) {
                $package = $1;
                $major = $4;
                $minor = $5;
                $micro = $6;
            }
        }

        die("No package found in $filename") unless defined $package;

        return ($package, $major, $minor, $micro);
    };

    my ($filename) = @_;

    return parseFile($parse, $filename);
}

# Writes new version number to configure.ac
#
sub updateConfigureAc {

    my $transform = sub {

        my ($srcfh, $srcfilename, $dstfh, $dstfilename, $major, $minor, $micro) = @_;

        while (<$srcfh>) {
            $_ =~ s/AC_INIT\(\[(\w+((-|_)\w+)?)\],\s*\[(\d+)\.(\d+)\.(\d+)\]/AC_INIT\(\[$1], \[$major.$minor.$micro\]/;
            print $dstfh $_;
        }
    };

    transformFile($transform, @_);
}


#########################################################################
#                                                                       #
#   ChangeLog                                                           #
#                                                                       #
#########################################################################

# Updates the ChangeLog file
#
sub updateChangeLog {

    my $transform = sub {

        my ($srcfh, $srcfilename, $dstfh, $dstfilename, $package_string) = @_;

        say $package_string;

        my ($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst) = localtime();
        $year = 1900 + $year;

        my @month = ('01', '02', '03', '04', '05', '06', '07', '08', '09', '10', '11', '12');

        # Create change-log header for new release
        print $dstfh "\n" .
                     "$package_string ($year-$month[$mon]-$mday)\n" .
                     "-"x length($package_string) . "\n" .
                     "\n" .
                     "    *\n" .
                     "\n";

        # Copy existing change-log entries
        while (<$srcfh>) {
            print $dstfh $_;
        }

        # We have to flush buffers before opening the editor.
        $dstfh->flush();

        # Open text editor for user to update ChangeLog file.
        system("\$EDITOR $dstfilename") == 0 or die;
    };

    my ($filename, $package, $new_major, $new_minor, $new_micro) = @_;

    my $new_package_string = "$package $new_major.$new_minor.$new_micro";

    transformFile($transform, $filename, $new_package_string);
}


#########################################################################
#                                                                       #
#   releases.json                                                       #
#                                                                       #
#########################################################################

# Updates the ChangeLog file
#
sub updateReleasesJson {

    my $transform = sub {

        sub StateString {
            my ($new_major) = @_;
            return 'production' if $new_major gt 0;
            return 'beta'       if $new_major eq 0;
            die;
        }

        sub ScopeString {
            my ($release_type) = @_;
            return 'major'  if $release_type eq 'major';
            return 'minor'  if $release_type eq 'minor';
            return 'bugfix' if $release_type eq 'micro';
            die;
        }

        my ($srcfh, $srcfilename, $dstfh, $dstfilename, $release_type, $new_major, $new_minor, $new_micro) = @_;

        my ($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst) = gmtime();
        $year = 1900 + $year;

        my @month = ('01', '02', '03', '04', '05', '06',
                     '07', '08', '09', '10', '11', '12');
        my @leadingzero = ('00',
                           '01', '02', '03', '04', '05', '06',
                           '07', '08', '09', '10', '11', '12',
                           '13', '14', '15', '16', '17', '18',
                           '19', '20', '21', '22', '23', '24',
                           '25', '26', '27', '28', '29', '30',
                           '31', '32', '33', '34', '35', '36',
                           '37', '38', '39', '40', '41', '42',
                           '43', '44', '45', '46', '47', '48',
                           '49', '50', '51', '52', '53', '54',
                           '55', '56', '57', '58', '59');

        my $state = StateString($new_major);
        my $scope = ScopeString($release_type);
        my $download = "https://github.com/picotm/picotm/releases/download/v$new_major.$new_minor.$new_micro/picotm-$new_major.$new_minor.$new_micro.tar.gz";
        my $published = "$year-$month[$mon]-$leadingzero[$mday]" .
                        "T$leadingzero[$hour]:$leadingzero[$min]:$leadingzero[$sec]+00:00";

        # Create out-commented header for new release
        print $dstfh "/*\n" .
                     "/*  1) Move this block to the first entry in the 'releases' array.\n" .
                     "/*  2) Update 'changes' field.\n" .
                     "/*  3) Remove comment lines.\n" .
                     "/*\n" .
                     "        {\n" .
                     "            \"version\": \"$new_major.$new_minor.$new_micro\",\n" .
                     "            \"state\": \"$state\",\n" .
                     "            \"scope\": \"$scope\",\n" .
                     "            \"changes\": \"\",\n" .
                     "            \"download\": \"$download\",\n" .
                     "            \"published\": \"$published\"\n" .
                     "        },\n" .
                     "*/\n";

        # Copy existing change-log entries
        while (<$srcfh>) {
            print $dstfh $_;
        }

        # We have to flush buffers before opening the editor.
        $dstfh->flush();

        # Open text editor for user to update ChangeLog file.
        system("\$EDITOR $dstfilename") == 0 or die;
    };

    my ($filename, $release_type, $new_major, $new_minor, $new_micro) = @_;

    transformFile($transform, $filename, $release_type, $new_major, $new_minor, $new_micro);
}


#########################################################################
#                                                                       #
#   git                                                                 #
#                                                                       #
#########################################################################

# Returns the name of the current git branch
#
sub getBranchName
{
    foreach (split(/\n/, `git branch`)) {
        next if not $_ =~ /^\*\s(.+)/; # name of current branch
        return $1;
    }
}

# Creates a git branch
#
sub createBranch
{
    my ($release_type, $cur_major, $cur_minor, $cur_micro, $new_major, $new_minor, $new_micro) = @_;

    my $master_branch  = 'master';
    my $release_branch = "v$cur_major.$cur_minor";

    my $branch = getBranchName() or die;

    die('Not on master branch') if $release_type eq 'major' and not $branch eq $master_branch;
    die('Not on master branch') if $release_type eq 'minor' and not $branch eq $master_branch;
    die("Not on branch '$release_branch'") if $release_type eq 'micro' and not $branch eq $release_branch;

    if ($branch eq $master_branch) {
        system("git checkout -b v$new_major.$new_minor") == 0 or die;
    }
}

# Commits all changes to git.
#
sub commitChanges
{
    my ($package_string) = @_;

    system("git commit -am \"$package_string\"") == 0 or die;
}

# Tags the new release
#
sub tagRelease
{
    my ($package_string, $major, $minor, $micro) = @_;

    system("git tag --sign -m \"$package_string\" v$major.$minor.$micro") == 0 or die;
}

# Returns the user's email address
#
sub getUserEmail
{
    my ($user_email, @tail) = split(/\n/, `git config --get user.email`);

    return $user_email;
}


#########################################################################
#                                                                       #
#   Signatures and Checksums                                            #
#                                                                       #
#########################################################################

sub signFile
{
    my ($filename, $username) = @_;

    system("gpg --output $filename.sig --local-user $username --detach-sig $filename") == 0 or die;
}


#########################################################################
#                                                                       #
#   Package Creation                                                    #
#                                                                       #
#########################################################################

sub generateRunConfigure
{
    my $generate = sub {

        my ($fh, $filename, $abssrcdir, $absbuilddir, $absinstalldir) = @_;

        print $fh "#!/bin/bash\n" .
                  "PREFIXDIR=\"$absinstalldir\"\n" .
                  "export CPPFLAGS=\"-I\${PREFIXDIR}/include\"\n" .
                  "export LDFLAGS=\"-L\${PREFIXDIR}/lib\"\n" .
                  "$abssrcdir/configure --prefix=\"\${PREFIXDIR}\"";

        chmod(0500, $filename);
    };

    my ($abssrcdir, $absbuilddir, $absinstalldir) = @_;

    # Never call this file 'configure' ! Doing so will package the
    # locally generated 'run-configure' script instead of the actual
    # configure script. The resulting package will not be buildable.
    # https://github.com/picotm/picotm/issues/256
    my $filename = "$absbuilddir/run-configure";

    generateFile($generate, $filename, $abssrcdir, $absbuilddir, $absinstalldir);

    return $filename;
}

sub createSourcePackage
{
    my $execute = sub {

        my ($package_name, $run_configure) = @_;

        system("$run_configure") == 0 or die;
        system('make distcheck') == 0 or die;

        # Later packages might depend on this one; so build and install.
        system('make') == 0 or die;
        system('make install') == 0 or die;

        return ("$package_name.tar.gz");
    };

    my ($package_name, $srcdir, $builddir, $installdir) = @_;

    my $abssrcdir = File::Spec->rel2abs($srcdir);
    my $absbuilddir = File::Spec->rel2abs($builddir);
    my $absinstalldir = File::Spec->rel2abs($installdir);

    my $run_configure = generateRunConfigure($abssrcdir, $absbuilddir, $absinstalldir);

    my ($filename) = executeInDir($execute, $absbuilddir, $package_name, $run_configure);

    copy("$absbuilddir/$filename", "./$filename");

    return $filename;
}

sub createDocPackage
{
    my $execute = sub {

        my ($package_name) = @_;

        system('OPTIPNG_FLAGS="-silent -o7 -zm1-9" make html') == 0 or die;
        system("mv doc/html $package_name") == 0 or die;
        system("tar -cf $package_name.tar $package_name/") == 0 or die;
        system("gzip -9 $package_name.tar") == 0 or die;

        return ("$package_name.tar.gz");
    };

    my ($package_name, $srcdir, $builddir, $installdir) = @_;

    my $abssrcdir = File::Spec->rel2abs($srcdir);
    my $absbuilddir = File::Spec->rel2abs($builddir);
    my $absinstalldir = File::Spec->rel2abs($installdir);

    my ($filename) = executeInDir($execute, $absbuilddir, $package_name);

    copy("$absbuilddir/$filename", "./$filename");
}

sub createPackages {

    my ($package, $major, $minor, $micro, $username) = @_;

    my $builddir = tempdir();
    my $installdir = tempdir();

    say "builddir: $builddir";
    say "installdir: $installdir";

    my $filename = createSourcePackage("$package-$major.$minor.$micro",
                                       './',
                                       "$builddir",
                                       $installdir);
    signFile($filename, $username);

    createDocPackage("$package-doc-$major.$minor.$micro", './', "$builddir", $installdir);
}


#########################################################################
#                                                                       #
#   Script                                                              #
#                                                                       #
#########################################################################

# Update version number for new release.
#
sub updateVersionNumber {

    my ($release_type, $cur_major, $cur_minor, $cur_micro) = @_;

    return ($cur_major + 1, 0,              0)              if ($release_type eq 'major');
    return ($cur_major,     $cur_minor + 1, 0)              if ($release_type eq 'minor');
    return ($cur_major,     $cur_minor,     $cur_micro + 1) if ($release_type eq 'micro');
}

# Get current version
#

my ($package, $cur_major, $cur_minor, $cur_micro) =
    parseConfigureAc('./configure.ac');

say "Package $package";
say "Current release: $cur_major.$cur_minor.$cur_micro";

my ($new_major, $new_minor, $new_micro) =
    updateVersionNumber($release_type, $cur_major, $cur_minor, $cur_micro);

say "Creating release: $new_major.$new_minor.$new_micro";

createBranch($release_type, $cur_major, $cur_minor, $cur_micro, $new_major, $new_minor, $new_micro);

my $new_package_string = "$package $new_major.$new_minor.$new_micro";

# Update and commit files for release
#

updateChangeLog('./ChangeLog', $package, $new_major, $new_minor, $new_micro);
updateReleasesJson('./releases.json', $release_type, $new_major, $new_minor, $new_micro);
updateConfigureAc('./configure.ac', $new_major, $new_minor, $new_micro);

commitChanges($new_package_string);

# Perform build and tests
#

$opt_username = getUserEmail() unless defined $opt_username;

createPackages($package, $new_major, $new_minor, $new_micro, $opt_username) unless $opt_package eq 0;

# Tag release
#

tagRelease($new_package_string, $new_major, $new_minor, $new_micro) unless $opt_tag eq 0;
