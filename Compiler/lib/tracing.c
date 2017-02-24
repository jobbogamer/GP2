/* Copyright 2015-2016 Christopher Bak

  This file is part of the GP 2 Compiler. The GP 2 Compiler is free software: 
  you can redistribute it and/or modify it under the terms of the GNU General
  Public License as published by the Free Software Foundation, either version 3
  of the License, or (at your option) any later version.

  The GP 2 Compiler is distributed in the hope that it will be useful, but 
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for 
  more details.

  You should have received a copy of the GNU General Public License
  along with the GP 2 Compiler. If not, see <http://www.gnu.org/licenses/>. */

#include "tracing.h"

/** File pointer to the trace file created by beginTraceFile(). */
FILE* tracefile = NULL;

/** Current depth of context nesting. Used to determine indent level. */
int context_depth = 0;

/** Indentation width in spaces for the tracefile. */
#define TRACE_INDENT 2

/** Convenience macro for printing to the tracefile. Automatically indents
the text to match the current context depth and indentation width. */
#define PrintToTracefile(text, ...)                                                            \
    do { fprintf(tracefile, "%*s" text, (context_depth * TRACE_INDENT), "", ##__VA_ARGS__); }  \
    while(0);

/** Shorthand for PrintToTracefile(). */
#define PTT PrintToTracefile


void beginTraceFile(char* tracefile_path, char* program_name, char* host_graph_name) {
    tracefile = fopen(tracefile_path, "w");
    if (tracefile == NULL) {
        /* If the tracefile couldn't be opened, print out the error and exit. */
        perror(tracefile_path);
        exit(1);
    }

    /* If we reach this point, the tracefile was opened, so insert the opening
    <trace> tag with the program name and the host graph name. */
    PTT("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
    PTT("<trace program=\"%s\" input_graph=\"%s\">\n", program_name, host_graph_name);
}

void finishTraceFile() {
    /* Add the final line to the file before closing it. */
    PTT("</trace>\n");
    fclose(tracefile);
}
