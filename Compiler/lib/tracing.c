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

    /* Since everything from this point onwards is inside the <trace> tag, we
    need to increase the context depth here. */
    context_depth += 1;
}

void finishTraceFile() {
    /* Add the final line to the file before closing it. We need to decrease
    context depth here because we're closing the <trace> context. */
    context_depth -= 1;
    PTT("</trace>\n");
    fclose(tracefile);
}

/**
    Internal function which returns the tag name used for a given
    TracingContext value.
*/
char* getContextTagName(TracingContext context) {
    switch (context) {
    case BACKTRACKING:
        return "backtracking";
    case RULE_CALL:
        return "rule";
    case RULE_MATCH:
        return "match";
    case RULE_APPLICATION:
        return "apply";
    case RULE_SET:
        return "ruleset";
    case PROCEDURE:
        return "procedure";
    case LOOP:
        return "loop";
    case LOOP_ITERATION:
        return "iteration";
    case IF_STATEMENT:
        return "if";
    case TRY_STATEMENT:
        return "try";
    case BRANCH_CONDITION:
        return "condition";
    case BRANCH_THEN:
        return "then";
    case BRANCH_ELSE:
        return "else";
    case OR_STATEMENT:
        return "or";
    case OR_LEFT:
        return "leftBranch";
    case OR_RIGHT:
        return "rightBranch";
    default:
        printf("Invalid tracing context value received: %d\n", context);
        return "";
    }
}

void traceBeginContext(TracingContext context, char* name) {
    /* First we need to determine the tag name to write. */
    char* tag_name = getContextTagName(context);

    /* The open tag should stay at the current indentation level, so write
    that to the file first before increasing the context depth.
    If there is a name attribute, we also need to add that to the tag. */
    if (name) {
        /* The attribute label for a ruleset needs to be "rules", not "name". */
        char* label = (context == RULE_SET) ? "rules" : "name";
        PTT("<%s %s=\"%s\">\n", tag_name, label, name);
    }
    else {
        PTT("<%s>\n", tag_name);
    }

    /* Now we can increase the context depth, because any future writes should
    be indented inside this tag, until this context is closed. */
    context_depth += 1;
}


void traceBeginSimpleContext(TracingContext context) {
    traceBeginContext(context, NULL);
}


void traceEndContext(TracingContext context) {
    /* At the end of a context, all we need to print is the closing tag, which
    cannot have any attributes. This means we just need to get the tag name and
    print it.
    However, first we have to decrease the context depth, because the closing
    tag has to be outdented from everything inside that tag. */
    context_depth -= 1;
    char* tag_name = getContextTagName(context);
    PTT("</%s>\n", tag_name);
}


