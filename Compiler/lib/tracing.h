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


typedef enum {
  PROGRAM = 0,
  BACKTRACKING,
  RULE_CALL,
  RULE_MATCH,
  RULE_APPLICATION,
  RULE_SET,
  PROCEDURE,
  LOOP,
  LOOP_ITERATION,
  IF_STATEMENT,
  TRY_STATEMENT,
  BRANCH_CONDITION,
  BRANCH_THEN,
  BRANCH_ELSE,
  OR_STATEMENT,
  OR_LEFT,
  OR_RIGHT
} TracingContext;


FILE* beginTraceFile(char* tracefile_path, char* program_name, char* host_graph_name);
void finishTraceFile(FILE* tracefile);

void traceBeginContext(TracingContext context, char* name, FILE* tracefile);
void traceEndContext(TracingContext context, FILE* tracefile);

void traceRuleMatch(Morphism* match, bool failed, FILE* tracefile);

void traceDeletedEdge(Edge* edge, FILE* tracefile);
void traceDeletedNode(Node* node, FILE* tracefile);
void traceRemarkedEdge(Edge* edge, MarkType old_mark, FILE* tracefile);
void traceRemarkedEdge(Node* node, MarkType old_mark, FILE* tracefile);
void traceRelabelledEdge(Edge* edge, HostLabel old_label, FILE* tracefile);
void traceRelabelledNode(Node* node, HostLabel old_label, FILE* tracefile);
void traceCreatedEdge(Edge* edge, FILE* tracefile);
void traceCreatedNode(Node* node, FILE* tracefile);
void traceChangeRootNode(Node* old_root, Node* new_root, FILE* tracefile);

void traceBreak(FILE* tracefile);
void traceSkip(FILE* tracefile);
void traceFail(FILE* tracefile);

#endif /* #ifndef INC_TRACING_H */
