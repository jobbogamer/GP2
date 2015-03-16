#ifndef OILRRT_H
#define OILRRT_H

extern void _HOST();
extern void GPMAIN();

void trace(int a, int b, int here);
typedef void (*Tracer)();

#define trav(n) (travs[n])
#define available(n) (!((n)->matched))

#define match(n) do {(n)->matched = 1;} while (0);  
#define unmatch(n) do {(n)->matched = 0;} while (0);

#define matchTarget(e) do { match( &elem(target(e)) ); } while (0)
#define unmatchTarget(e) do { unmatch( &elem(target(e)) ); } while (0)
#define matchSource(e) do { match( &elem(source(e)) ); } while (0)
#define unmatchSource(e) do { unmatch( &elem(source(e)) ); } while (0)

#define matchLoop(n) do { \
	(n)->matchedLoops++; } while (0)
#define unmatchLoop(n) do { \
	(n)->matchedLoops--; } while (0)
#define availableLoops(n) (loopdeg(n) - (n)->matchedLoops)

typedef struct Trav {
	const int first;
	const int last;
	int o, i, l, r;

	int cur;
	int next;

	EdgeId edge;
	NodeId match;

} Trav;

typedef struct SearchPlan {
	const int first;
	const int last;
} SearchPlan;

typedef int TravId;

extern int searchSpaces[];
extern Trav travs[];
extern int success;

void search(Trav *t);
void followOutEdge(Trav *from, Trav *to);
void followInEdge(Trav *to, Trav *from);
void edgeBetween(Trav *from, Trav *to, int negate);
void reset(Trav *t);

#endif
