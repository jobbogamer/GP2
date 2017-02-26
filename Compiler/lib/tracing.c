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

/** Structure to hold one element of the context stack. */
typedef struct TracingContext {
    char* context_type;
    struct TracingContext* previous;
} TracingContext;

/** Stack of current contexts. This is required since some statements, namely
break and fail, can exit more than one context at a time, and the compiled
program does not keep track of the contexts it is in, because it doesn't need
to for normal execution.
It's valid to use a stack to determine the current nesting of contexts, because
XML tags cannot overlap, e.g. <a><b></a></b> is invalid XML. Inner tags must be
closed before outer ones, so a stack works perfectly. It also means that the
traceEndContext() function doesn't need an argument.
Each time a context is started, it should be added to the stack, and each time
a context ends, it should be removed. */
TracingContext* context_stack = NULL;

/** Current depth of context stack. Used to determine indent level. */
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


/** Internal function for adding an item to the TracingContext stack.
Creates a new TracingContext object and adds it to the stack.
There must be a matching number of popContextStack() calls to avoid leaking. */
void pushContextStack(char* context_type) {
    /* Create the new element and set its fields. */
    TracingContext* element = (TracingContext*) malloc(sizeof(TracingContext));
    element->context_type = context_type;
    element->previous = context_stack;

    /* Add it to the stack. Really this just means "update the stack pointer
    to point at this element", since each element points to the previous item
    on the stack. We also update the context depth. */
    context_stack = element;
    context_depth += 1;
}


/** Internal function for popping an item from the TracingContext stack.
Frees the top element of the stack and moves the pointer to the next item. Then
the context_type of the popped element is returned. */
char* popContextStack() {
    /* Get the context_type so it can be returned later. */
    char* context_type = context_stack->context_type;

    /* Take a pointer to the top of the stack so it can be freed after updating
    the stack pointer. */
    TracingContext* top_element = context_stack;

    /* Update the stack pointer to point at the next item down and decrease
    the current depth level. */
    context_stack = context_stack->previous;
    context_depth -= 1;

    /* Free the memory and return the context_type. */
    free(top_element);
    return context_type;
}


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
    need to add it to the stack. */
    pushContextStack("trace");
}

void finishTraceFile() {
    /* Here we add the closing </trace> tag to the tracefile. We need to pop 
    the stack first to decrease the depth, so that the tag is printed at the
    correct indentation level. */
    popContextStack();
    PTT("</trace>\n");
    fclose(tracefile);
}


void traceBeginLabelledContext(char* context_type, char* label, char* label_value) {
    /* Small guard against NULL values: if label is given but label_value is
    NULL, set label_value to the empty string. */
    if (label && !label_value) {
        label_value = "";
    }

    /* The open tag should stay at the current indentation level, so write
    that to the file first before increasing the context depth.
    If there is a name attribute, we also need to add that to the tag. */
    if (label) {
        PTT("<%s %s=\"%s\">\n", context_type, label, label_value);
    }
    else {
        PTT("<%s>\n", context_type);
    }

    /* Push the newly started context onto the stack. */
    pushContextStack(context_type);
}


void traceBeginNamedContext(char* context_type, char* name) {
    traceBeginLabelledContext(context_type, "name", name);
}


void traceBeginContext(char* context_type) {
    traceBeginLabelledContext(context_type, NULL, NULL);
}


void traceEndContext() {
    /* At the end of a context, all we need to print is the closing tag, which
    cannot have any attributes. This means we just need to get the tag name by
    popping the top of the context stack. */
    char* context_type = popContextStack();
    PTT("</%s>\n", context_type);
}


void traceSkip() {
    /* Since skip does absolutely nothing, all we have to do is print that skip
    was used. Nothing about the program's state changes. */
    PTT("<skip />\n");
}


void traceBreak() {
    PTT("<break />\n");

    /* The break statement quits the current loop. This means that it ends all
    contexts back to the most recent <loop> context. Iterate over the context
    stack, closing each context, until we find the first <loop> context. The
    <loop> context is always closed by the runtime program, so we don't need
    to close that one. */
    char* context_type;
    while (strcmp(context_stack->context_type, "loop") != 0) {
        context_type = popContextStack();
        PTT("</%s>\n", context_type);
    }
}


void traceFail() {
    PTT("<fail />\n");

    /* The fail statement essentially aborts the program. We need to close
    every context that's currently open, *except* the outermost <trace> context
    because that gets closed in finishTraceFile(), which does still get called
    when fail is invoked. We're going to pop and close contexts from the stack
    until the context depth reaches 1, because that's the trace context. */
    char* context_type;
    while (context_depth > 1) {
        context_type = popContextStack();
        PTT("</%s>\n", context_type);
    }
}
