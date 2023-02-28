# -*- python -*-
#                           Package   : omniidl
# output.py                 Created on: 2000/8/10
#			    Author    : David Scott (djs)
#
#    Copyright (C) 2002-2011 Apasphere Ltd
#    Copyright (C) 1999 AT&T Laboratories Cambridge
#
#  This file is part of omniidl.
#
#  omniidl is free software; you can redistribute it and/or modify it
#  under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see http://www.gnu.org/licenses/
#
# Description:
#   
#   Routines for managing output

"""Routines for managing output"""

# Wrapper around the standard library file opening routines, keeps track of
# which files have been created by the backend so they can be cleaned up on
# error.
createdFiles = []

def createFile(filename):
    try:
        if filename in createdFiles:
            file = open(filename, "a")
        else:
            file = open(filename, "w")
            createdFiles.append(filename)
        return file

    except IOError:
        import sys
        sys.stderr.write("omniidl: Cannot open file '%s' for writing.\n" %
                         filename)
        sys.exit(1)

def listAllCreatedFiles():
    return createdFiles


# Stream is a wrapper around a python file stream
# StringStream is a string buffer with a stream interface
#
# Holes in output template string can consist of:
#   Strings - inserted as is
#   StringStreams - converted into strings and inserted
#   Integers - converted into strings (using str) and inserted
#   Functions - stream is updated to reflect the current indent level
#      and the function is applied (permitting template driven code
#      rather than code-driving-template style)
#
# Multiline strings are inserted at the same indent level eg
#
# ~~~~~          ~~~~~
#   @foo@   ->     ~~~~~~
#                  ~~~~~~
# ~~~~~          ~~~~~              
#
# Entire lines are buffered and if they consist of a preprocessor
# directive ( ^\s*# ) the preceeding whitespace is trimmed.
#
# Note that a line which is empty apart from a hole is still a complete
# line even if the hole itself is empty. This commonly causes large blocks of
# empty lines in the output. For aesthetic effect, runs of more than 1 empty
# line are removed.

def dummy(): pass

StringType = type("")
FuncType   = type(dummy)

class Stream:
    def __init__(self, file, indent_size = 2):
        self.file        = file
        self.indent_size = indent_size
        self.indent      = 0
        self.do_indent   = 1

    def inc_indent(self):   self.indent = self.indent + self.indent_size
    def dec_indent(self):   self.indent = self.indent - self.indent_size
    def reset_indent(self):
        self.indent = 0

    def out(self, text, ldict={}, **dict):
        """Output a multi-line string with indentation and @@ substitution."""

        dict.update(ldict)

        pos    = 0
        tlist  = text.split("@")
        ltlist = len(tlist)
        i      = 0
        while i < ltlist:

            # Output plain text
            pos = self.olines(pos, self.indent, tlist[i])

            i = i + 1
            if i == ltlist: break

            # Evaluate @ expression
            try:
                expr = dict[tlist[i]]

            except KeyError:
                # If a straight look-up failed, try evaluating it
                if tlist[i] == "":
                    expr = "@"
                else:
                    expr = eval(tlist[i], globals(), dict)

            if type(expr) is StringType:
                pos = self.olines(pos, pos, expr)
                
            elif type(expr) is FuncType:
                oindent = self.indent
                self.indent = pos
                expr()
                self.indent = oindent
            else:
                pos = self.olines(pos, pos, str(expr))

            i = i + 1

        self.odone()

    def niout(self, text, ldict={}, **dict):
        """Output a multi-line string without indentation."""

        dict.update(ldict)

        pos    = 0
        tlist  = text.split("@")
        ltlist = len(tlist)
        i      = 0
        while i < ltlist:

            # Output plain text
            pos = self.olines(pos, 0, tlist[i])

            i = i + 1
            if i == ltlist: break

            # Evaluate @ expression
            try:
                expr = dict[tlist[i]]

            except KeyError:
                # If a straight look-up failed, try evaluating it
                if tlist[i] == "":
                    expr = "@"
                else:
                    expr = eval(tlist[i], globals(), dict)

            if type(expr) is StringType:
                pos = self.olines(pos, pos, expr)
                
            elif type(expr) is FuncType:
                oindent = self.indent
                self.indent = pos
                expr()
                self.indent = oindent
            else:
                pos = self.olines(pos, pos, str(expr))

            i = i + 1

        self.odone()

    def olines(self, pos, indent, text):
        istr  = " " * indent
        write = self.file.write

        stext = text.split("\n")
        lines = len(stext)
        line  = stext[0]

        if self.do_indent:
            if line and line[0] == "#":
                pos = 0
            else:
                pos = indent
                write(istr)

        write(line)

        for i in range(1, lines):
            line = stext[i]
            write("\n")
            if line:
                if line[0] == "#":
                    pos = 0
                else:
                    pos = indent
                    write(istr)
                write(line)

        if lines > 1 and not line: # Newline at end of text
            self.do_indent = 1
            return self.indent

        self.do_indent = 0
        return pos + len(line)

    def odone(self):
        self.file.write("\n")
        self.do_indent = 1

    def close(self):
        self.file.close()


class StringStream(Stream):
    """Writes to a string buffer rather than a file."""
    def __init__(self, indent_size = 2):
        Stream.__init__(self, self, indent_size)
        self.buffer = []

    def write(self, text):
        self.buffer.append(text)

    def __str__(self):
        return "".join(self.buffer)
