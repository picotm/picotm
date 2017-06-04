#!/usr/bin/env perl

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

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

my $release_type = undef;

GetOptions('release-type=s' =>  \$release_type) # string: 'major', 'minor',
                                                #         'micro'
    or die;

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


#########################################################################
#                                                                       #
#   Package Creation                                                    #
#                                                                       #
#########################################################################

sub generateConfigure
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

    my $filename = "$absbuilddir/configure";

    generateFile($generate, $filename, $abssrcdir, $absbuilddir, $absinstalldir);

    return $filename;
}

sub createSourcePackage
{
    my $generate = sub {

        my ($fh, $filename, $abssrcdir, $absbuilddir, $absinstalldir) = @_;

        print $fh "#!/bin/bash\n" .
                  "PREFIXDIR=\"$absinstalldir\"\n" .
                  "export CPPFLAGS=\"-I\${PREFIXDIR}/include\"\n" .
                  "export LDFLAGS=\"-L\${PREFIXDIR}/lib\"\n" .
                  "$abssrcdir/configure --prefix=\"\${PREFIXDIR}\"";
    };

    my $execute = sub {

        my ($package_name, $configure) = @_;

        system("$configure") == 0 or die;
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

    my $configure = generateConfigure($abssrcdir, $absbuilddir, $absinstalldir);

    my ($filename) = executeInDir($execute, $absbuilddir, $package_name, $configure);

    copy("$absbuilddir/$filename", "./$filename");
}

sub createDocPackage
{
    my $execute = sub {

        my ($package_name) = @_;

        system('make html') == 0 or die;
        system('find doc/html -type f -name "*.png" | xargs optipng -o7 -zm1-9') == 0 or die;
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

    my ($package, $major, $minor, $micro) = @_;

    my $builddir = tempdir();
    my $installdir = tempdir();

    say "builddir: $builddir";
    say "installdir: $installdir";

    mkdir "$builddir/lib";
    createSourcePackage("$package-$major.$minor.$micro", 'lib/', "$builddir/lib", $installdir);

    mkdir "$builddir/tests";
    createSourcePackage("$package-tests-$major.$minor.$micro", 'tests/', "$builddir/tests", $installdir);

    createDocPackage("$package-doc-$major.$minor.$micro", 'lib/', "$builddir/lib", $installdir);
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
    parseConfigureAc('lib/configure.ac');

say "Package $package";
say "Current release: $cur_major.$cur_minor.$cur_micro";

my ($new_major, $new_minor, $new_micro) =
    updateVersionNumber($release_type, $cur_major, $cur_minor, $cur_micro);

say "Creating release: $new_major.$new_minor.$new_micro";

createBranch($release_type, $cur_major, $cur_minor, $cur_micro, $new_major, $new_minor, $new_micro);

my $new_package_string = "$package $new_major.$new_minor.$new_micro";

# Update and commit files for release
#

updateChangeLog('lib/ChangeLog', $package, $new_major, $new_minor, $new_micro);
updateChangeLog('tests/ChangeLog', "$package-tests", $new_major, $new_minor, $new_micro);

updateConfigureAc('lib/configure.ac', $new_major, $new_minor, $new_micro);
updateConfigureAc('tests/configure.ac', $new_major, $new_minor, $new_micro);

commitChanges($new_package_string);

# Perform build and tests
#

createPackages($package, $new_major, $new_minor, $new_micro);

# Tag release
#

tagRelease($new_package_string, $new_major, $new_minor, $new_micro)