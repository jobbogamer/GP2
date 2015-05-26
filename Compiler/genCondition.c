#include "genCondition.h"

/* For eaech predicate in the condition, generate a boolean value 'bx', where x
 * is the ID of the predicate. The variables are initialised in such a way that
 * the condition always evaluates to true, so that the condition isn't erroneously
 * falsified when one of these variables is modified by the evaluation of a 
 * predicate. */
void generateConditionVariables(Condition *condition)
{
   static int bool_count = 0;
   switch(condition->type)
   {
      /* Booleans representing positive predicates are initialised with true. */
      case 'e':
           PTF("bool b%d = true;\n", bool_count++);
           break;

      /* Booleans representing 'not' predicates are initialised with false. */
      case 'n':
           PTF("bool b%d = false;\n", bool_count++);
           break;

      case 'a':
      case 'o':
           generateConditionVariables(condition->left_predicate, variables, nested);
           generateConditionVariables(condition->right_predicate, variables, nested);
           break;

      default:
           print_to_log("Error (generateConditionVariables): Unexpected condition "
                        "type '%c'.\n", condition->type);
           break;
   }
}

/* Generates a boolean expression over the variables generated by the above function.
 * The tree structure of the condition is used to print the correct expression. */
void generateConditionEvaluator(Condition *condition, bool nested)
{
   PTF("static bool evaluateCondition(void)\n");
   PTF("{\n");
   PTFI("return (", 3);
   static int bool_count = 0;
   switch(condition->type)
   {
      case 'e':
           PTF("b%d", bool_count++);
           break;

      case 'n':
           PTF("!b%d", bool_count++);
           break;

      case 'a':
           if(nested) PTF("(");
           generateConditionCode(condition->left_predicate, variables, nested);
           PTF(" && ");
           generateConditionCode(condition->right_predicate, variables, nested);
           if(nested) PTF(")");
           break;
           
      case 'o':
           if(nested) PTF("(");
           generateConditionCode(condition->left_predicate, variables, nested);
           PTF(" || ");
           generateConditionCode(condition->right_predicate, variables, nested);
           if(nested) PTF(")");
           break;

      default:
           print_to_log("Error (generateConditionExpression): Unexpected condition "
                        "type '%c'.\n", condition->type);
           break;
   }
   PTF(");\n");
   PTF("}\n\n");
}

generatePredicateEvaluators(Condition *condition)
{
   switch(condition->type)
   {
      case 'e':
           PTF("b%d", bool_count++);
           break;

      case 'n':
           PTF("!b%d", bool_count++);
           break;

      case 'a':
      case 'o':
           generatePredicateEvaluators(condition->left_predicate);
           generatePredicateEvaluators(condition->right_predicate);
           break;

      default:
           print_to_log("Error (generatePredicateEvaluators): Unexpected condition "
                        "type '%c'.\n", condition->type);
           break;
   }
}

/* Writes a function that evaluates a predicate. The generated function checks
 * if all appropriate nodes and variables are instantiated. If so, it sets the
 * appropriate runtime boolean value to the result of the predicate's evalution
 * and returns true. Otherwise, it returns false. */
void generatePredicateCode(Rule *rule, Predicate *predicate)
{
   PTF("static bool evaluatePredicate%d(void)\n", predicate->bool_id);
   PTF("{\n");
   int index;
   /* Generate code for any nodes that participate in this predicate. */
   for(index = 0; index < rule->lhs->node_index; index++)
   {
      RuleNode *node = getRuleNode(rule->lhs, index);
      if(node->predicates == NULL) continue;
      int p;
      for(p = 0; p < node->predicate_count; p++)
      {
         if(node->predicates[p] == predicate)
         {
            PTFI("int n%d = lookupNode(morphism, %d);\n", 3, index, index);
            PTFI("/* If the node is not yet matched by the morphism, return false. */\n", 3);
            PTFI("if(n%d == -1) return false;\n\n", 3, index);
            break;
         }
      }
   }
   /* Generate code for any variables that participate in this predicate. */
   for(index = 0; index < rule->variable_index; index++)
   {
      Variable variable = rule->variables[index];
      if(variable.predicates == NULL) continue;
      int p;
      for(p = 0; p < variable.predicate_count; p++)
      {
         if(variable.predicates[p] == predicate)
         {
            string name = variable.name;
            PTFI("Assignment *assignment_%s = lookupVariable(morphism, \"%s\");\n",
                 3, name, name);
            PTFI("/* If the variable is not yet assigned, return false. */\n", 3);
            PTFI("if(assignment_%s == NULL) return false;\n", 3, name);
            switch(variable.type)
            {
               case INTEGER_VAR:
                    PTFI("int %s_var = getIntegerValue(\"%s\", morphism);\n\n", 3, 
                         name, name);
                    break;

               case CHARACTER_VAR:
               case STRING_VAR:
                    PTFI("string %s_var = getStringValue(\"%s\", morphism);\n\n", 3, 
                         name, name);
                    break;

               case ATOM_VAR:
                    PTFI("union { int num; string str; } %s_var;\n", 3, name);
                    PTFI("if(assignment_%s->type == INTEGER_VAR) "
                         "%s_var.num = getIntegerValue(\"%s\", morphism);\n", 3, 
                         name, name, name);
                    PTFI("else %s_var.str = getStringValue(\"%s\", morphism);\n\n", 3,
                         name, name);
                    break;
                  
               case LIST_VAR:
                    break;
               
               default:
                    print_to_log("Error (generateVariableCode): Unexpected type %d\n",
                                 variable.type);
                    break;
            }
            break;
         }
      }
   }
   int list_count = 0;
   switch(predicate->type)
   {
      case INT_CHECK:
           PTFI("if(assignment_%s->type == INT_VAR) b%d = true;\n", 3,
                predicate->variable, predicate->bool_id);
           PTFI("else b%d = false;\n", 3, predicate->bool_id);
           break;

      case CHAR_CHECK:
           PTFI("if(assignment_%s->type == STRING_VAR &&\n", 3, predicate->variable);
           PTFI("strlen(assignment_%s->value[0].string) == 1)\n", 6, predicate->variable);
           PTFI("b%d = true;\n", 6, predicate->bool_id);
           PTFI("else b%d = false;\n", 3, predicate->bool_id);
           break;

      case STRING_CHECK:
           PTFI("if(assignment_%s->type == STRING_VAR) b%d = true;\n",
                3, predicate->variable, predicate->bool_id);
           PTFI("else b%d = false;\n", 3, predicate->bool_id);
           break;

      case ATOM_CHECK:
           PTFI("if(assignment_%s->type != LIST_VAR) b%d = true;\n",
                3, predicate->variable, predicate->bool_id);
           PTFI("else b%d = false;\n", 3, predicate->bool_id);
           break;

      case EDGE_PRED:
      {
           int source = predicate->edge_pred.source;    
           int target = predicate->edge_pred.target;    
           PTFI("Node *source = getNode(host, n%d);\n", 3, source);
           PTFI("bool edge_found = false;\n", 3);
           PTFI("int counter;\n", 3);
           PTFI("for(counter = 0; counter < source->out_index; counter++)\n", 3);
           PTFI("{\n", 3);
           PTFI("Edge *edge = getEdge(host, getOutEdge(source, counter));\n", 6);
           PTFI("if(edge != NULL && edge->target == %d)\n", 6, target);
           if(predicate->edge_pred.label != NULL)
           { 
              PTFI("{\n", 6);
              PTFI("Label label;\n", 9);
              /* bool "node" argument taken from the caller. 
               * Also add some argument to mark that this is coming from a predicate. */
              generateLabelEvaluationCode(*(predicate->edge_pred.label), false, 
                                          list_count++, 0, 9);
              PTFI("if(equalRuleLabels(label, edge->label))\n", 9);
              PTFI("{\n", 9);
              PTFI("b%d = true;\n", 12, predicate->bool_id);
              PTFI("edge_found = true;\n", 12);
              PTFI("break;\n", 12);
              PTFI("}\n", 9);
              PTFI("}\n", 6);
           }
           else
           {
              PTFI("{\n", 6);
              PTFI("b%d = true;\n", 9, predicate->bool_id);
              PTFI("edge_found = true;\n", 9);
              PTFI("break;\n", 9);
              PTFI("}\n", 6);
           }
           PTFI("if(!edge_found) b%d = false;\n", 6, predicate->bool_id);
           PTFI("}\n", 3);
           break;
      }
      case EQUAL:
      case NOT_EQUAL:
           PTFI("Label left_label, right_label;\n", 3);
           generateLabelEvaluationCode(predicate->comparison.left_label, false, 
                                       list_count++, 1, 3);
           generateLabelEvaluationCode(predicate->comparison.right_label, false, 
                                       list_count++, 1, 3);
           if(predicate->type == EQUAL) PTFI("if(equalHostLabels(label0, label1));\n", 3);
           if(predicate->type == NOT_EQUAL) PTFI("if(equalHostLabels(label0, label1));\n", 3);
           PTFI("b%d = true;\n", 6, predicate->bool_id);
           
      case GREATER:
      case GREATER_EQUAL:
      case LESS:
      case LESS_EQUAL:
           PTFI("if(", 3);
           generateIntExpression(predicate->atom_comp.left_atom, 1, false);
           if(predicate->type == GREATER) PTF(" > ");
           if(predicate->type == GREATER_EQUAL) PTF(" >= ");
           if(predicate->type == LESS) PTF(" < ");
           if(predicate->type == LESS_EQUAL) PTF(" <= ");
           generateIntExpression(predicate->atom_comp.right_atom, 1, false);
           PTF(") b%d = true;\n", predicate->bool_id);

      default:
           print_to_log("Error (generatePredicateCode): Unexpected type %d.\n", 
                        predicate->type);
           break;
   }
   PTFI("return true;\n", 3);
   PTF("}\n");
}
