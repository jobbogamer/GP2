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

/** Macro for appending to the current line of the tracefile, without adding
indentation. Primarily used for writing the contents of GP2 lists. */
#define AppendToTracefile(text, ...)                \
    do { fprintf(tracefile, text, ##__VA_ARGS__); } \
    while(0);

/** Shorthand for PrintToTracefile(). */
#define PTT PrintToTracefile

/** Shorthand for AppendToTracefile(). */
#define ATT AppendToTracefile


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


/** Internal function which returns the context_type from the top of the stack
without removing it from the stack. */
char* peekContextStack() {
    return context_stack->context_type;
}


/** Internal function for printing a GP2 list to the tracefile. */
void traceGP2List(HostList* list) {
    /* Pointer to current item in the list */
    HostListItem* current = NULL;

    /* Check for an empty assignment, given by a NULL list. */
    if (list) { current = list->first; }

    /* A GP2 list is a linked list of HostListItem structs. Here
    we iterate over them until there is no next item, which
    means we have reached the end of the list. (This may be true
    from the beginning, if the list was empty. */
    while (current) {
        /* Append this item to the tracefile. If it's not the first
        one, we need to add a colon beforehand. */
        char* colon = (current->prev) ? ":" : "";
        switch(current->atom.type) {
        case 'i': /* integer item */
            ATT("%s%d", colon, current->atom.num);
            break;
        
        case 's': /* string item */
            /* Strings will be surrounded by double quotes. */
            ATT("%s\"%s\"", colon, current->atom.str);
            break;

        default:
            printf("Unknown variable type %c in list", current->atom.type);
            break;
        }

        /* Move on to the next item in the list. */
        current = current->next;
    }
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


void traceRuleMatch(Morphism* morphism, bool success) {
    /* First we need to start a context called match. We need to print whether
    the match failed, since there are two situations where the morphism will be
    empty:
    1. If the rule failed to match (failed = true)
    2. If the LHS of the rule is empty (failed = false)
    We need to be able to distinguish between these two cases. */
    char* success_value = (success) ? "true" : "false";
    traceBeginLabelledContext("match", "success", success_value);

    /* If we know the match failed, we can skip this part, since we know the
    match will be empty. A null morphism pointer also means the match is empty. */
    if (success && morphism) {
        int index, id;

        /* Iterate over the nodes in the morphism and add them to the trace. */
        for (index = 0; index < morphism->nodes; index++) {
            id = morphism->node_map[index].host_index;
            PTT("<node id=\"%d\" />\n", id);
        }

        /* Iterate over the edges and and those. */
        for (index = 0; index < morphism->edges; index++) {
            id = morphism->edge_map[index].host_index;
            PTT("<edge id=\"%d\" />\n", id);
        }

        /* Finally, iterate over the variable assignments found for this match.
        These are indexed by "variable ID", not by their position in the list. */
        for (id = 0; id < morphism->variables; id++) {
            Assignment* assignment = &(morphism->assignment[id]);
            switch(assignment->type) {
                case 'i': /* integer assignment */
                    PTT("<variable id=\"%d\" type=\"integer\" value=\"%d\" />\n", id, assignment->num);
                    break;

                case 's': /* string assignment */
                    PTT("<variable id=\"%d\" type=\"string\" value=\"%s\" />\n", id, assignment->str);
                    break;

                case 'l': /* list assignment */
                    /* Print the start of the variable tag to the file, but do not
                    finish it. Each item in the list will be added one by one, and
                    the tag will be finished at the end. Note that we use a single
                    quote for the value attribute, so that double quotes can be used
                    to represent strings. */
                    PTT("<variable id=\"%d\" type=\"list\" value='", id);

                    traceGP2List(assignment->list);

                    /* Now we have reached the end of the list, so finish the
                    line in the tracefile. */
                    ATT("' />\n");
                    break;

                default:
                    printf("Unknown variable type %c in morphism", morphism->assignment[index].type);
                    break;
            }
        }
    }

    /* Now we can end the match context. */
    traceEndContext(/* match */);
}


void traceBeginRuleApplicationContext() {
    traceBeginContext("apply");
}


void traceEndRuleApplicationContext() {
    /* Since the generated code for deleting/relabelling/etc doesn't know if
    it's the last operation in the rule application, it cannot close the
    <deleted> or <created> or etc context, so we have to do that here. 
    There will only be one context to close other than <apply>, because there
    is no recursion or nesting in a rule application. */
    if (strcmp("apply", peekContextStack()) != 0) {
        traceEndContext();
    }

    /* Now we end the <apply> context. */
    traceEndContext();
}


void traceDeletedEdge(Edge* edge) {
    /* If the current context is not a <deleted> context, start one here.
    This relies on the order things are done during a rule application, and
    that all deletions (both edges and nodes) are done at the same time. 
    See generateApplicationCode() in genRule.c for info. */
    if (strcmp("deleted", peekContextStack()) != 0) {
        traceBeginContext("deleted");
    }

    /* Now simply print the details of the edge. We print all the details so
    that if we want to step backwards in the trace, we can recreate the edge
    as it was before it was deleted.
    Note we haven't finished the XML tag so that traceGP2List() can append
    the edge's label to the tag. Also note that we use single quotes for the
    label attribute, so that we can use double quotes to represent strings. */
    PTT("<edge id=\"%d\" source=\"%d\" target=\"%d\" mark=\"%d\" label='",
        edge->index, edge->source, edge->target, edge->label.mark);
    traceGP2List(edge->label.list);
    ATT("' />\n");
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


void traceFail(bool escape_context, bool main_body) {
    PTT("<fail />\n");

    if (escape_context) {
        if (main_body) {
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
        else {
            /* In the non-terminating case, we still need to close some contexts
            automatically, but not all of them. We need to stop if we see a
            <condition> context, since that's the exit point when fail is used
            in an if or try body. */
            char* context_type;
            while (strcmp(context_stack->context_type, "condition") != 0) {
                context_type = popContextStack();
                PTT("</%s>\n", context_type);
            }
        }
    }
}
