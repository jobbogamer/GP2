/* ///////////////////////////////////////////////////////////////////////////

  Copyright 2015-2016 Christopher Bak

  This file is part of the GP 2 Compiler. The GP 2 Compiler is free software: 
  you can redistribute it and/or modify it under the terms of the GNU General
  Public License as published by the Free Software Foundation, either version 3
  of the License, or (at your option) any later version.

  The GP 2 Compiler is distributed in the hope that it will be useful, but 
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for 
  more details.

  You should have received a copy of the GNU General Public License
  along with the GP 2 Compiler. If not, see <http://www.gnu.org/licenses/>.

  ==============
  Tracing Module
  ==============
                             
  Contains functions for tracing the execution of a compiled GP2 program.

/////////////////////////////////////////////////////////////////////////// */

#ifndef INC_TRACING_H
#define INC_TRACING_H


#include "globals.h"
#include "morphism.h"
#include "graph.h"
#include "label.h"


/**
    Creates a new file at the given path and starts the program trace by adding
    the opening <trace> tag to the file.
*/
void beginTraceFile(char* tracefile_path, char* program_name, char* host_graph_name);

/**
    Adds the closing </trace> tag to the tracefile and closes the file pointer.
*/
void finishTraceFile();

/**
    Starts a new context in the tracefile. This means an opening XML tag is
    added and, until this context is closed, any future tracing will be nested
    inside that tag.
    A "labelled" context is one which has an attribute in the XML tag. The
    parameters are interpreted to create a tag as follows:
    <context_type label="label_value">
    Contexts must be closed using traceEndContext() for the tracefile to be valid.
*/
void traceBeginLabelledContext(char* context_type, char* label, char* label_value);

/**
    Convenience function for traceBeginLabelledContext(). Defaults the 'label'
    parameter to "name", resulting in a tag as follows:
    <context_type name="_name_"> (where _name_ is the value of the second argument
    to this function).
    Contexts must be closed using traceEndContext() for the tracefile to be valid.
*/
void traceBeginNamedContext(char* context_type, char* name);

/**
    Convenience function for traceBeginLabelledContext(). Defaults both the 'label'
    and 'label_value' parameters to NULL, resulting in a tag with no attribute:
    <context_type>
    Contexts must be closed using traceEndContext() for the tracefile to be valid.
*/
void traceBeginContext(char* context_type);

/**
    Closes the most recently opened tracing context by adding a closing XML tag to
    the tracefile. No argument is required since this module will keep track of
    the current context and always closes contexts in LIFO order.
*/
void traceEndContext();

void traceRuleMatch(Morphism* match, bool success);

void traceDeletedEdge(Edge* edge);
void traceDeletedNode(Node* node);
void traceRemarkedEdge(Edge* edge, MarkType new_mark);
void traceRemarkedNode(Node* node, MarkType new_mark);
void traceRelabelledEdge(Edge* edge, HostLabel new_label);
void traceRelabelledNode(Node* node, HostLabel new_label);
void traceCreatedEdge(Edge* edge);
void traceCreatedNode(Node* node);
void traceSetRootNode(Node* node);
void traceRemoveRootNode(Node* node);

/**
    Traces the use of a GP2 break statement in the program. Since break exits
    the current loop, this function automatically closes all contexts up to,
    but *not including* the most recent loop context.
    This means that there should always be a traceEndContext() call after a 
    GP2 loop, whether a break statement is used or not.
*/
void traceBreak();

/**
    Traces the use of a GP2 skip statement in the program.
*/
void traceSkip();

/**
    Traces the use of a GP2 fail statement in the program.
    
    If fail is called from the main body of the program pass true for both arguments,
    and this function will automatically close all contexts except the
    outermost one, the <trace> tag created by beginTraceFile(). finishTraceFile()
    still needs to be called after a fail statement is used in the program.
    
    If fail is called inside an if or try condition, or in a loop, pass true as
    the first argument and false as the second, and this function will close the
    necessary contexts, up to the <condition> context.

    If fail is called inside a loop, pass false for both arguments, since the loop
    body is not exited early when calling fail.
*/
void traceFail(bool escape_context, bool main_body);

#endif /* #ifndef INC_TRACING_H */
