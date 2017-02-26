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

void traceBeginLabelledContext(char* context_type, char* label, char* label_value);
void traceBeginNamedContext(char* context_type, char* name);
void traceBeginContext(char* context_type);
void traceEndContext();

void traceRuleMatch(Morphism* match, bool failed);

void traceDeletedEdge(Edge* edge);
void traceDeletedNode(Node* node);
void traceRemarkedEdge(Edge* edge, MarkType old_mark);
void traceRemarkedNode(Node* node, MarkType old_mark);
void traceRelabelledEdge(Edge* edge, HostLabel old_label);
void traceRelabelledNode(Node* node, HostLabel old_label);
void traceCreatedEdge(Edge* edge);
void traceCreatedNode(Node* node);
void traceChangeRootNode(Node* old_root, Node* new_root);

void traceBreak();
void traceSkip();
void traceFail();

#endif /* #ifndef INC_TRACING_H */
