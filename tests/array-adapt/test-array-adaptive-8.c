// RUN: %clam -m32 --crab-inter --crab-track=arr --crab-dom=int --crab-check=assert --crab-sanity-checks  --lower-unsigned-icmp --crab-widening-jump-set=20  --llvm-pp-loops "%s" 2>&1 | OutputCheck %s
// CHECK: ^2  Number of total safe checks$
// CHECK: ^0  Number of total warning checks$
// XFAIL: *

// We need here llvm-seahorn. The problem is that "i" (the loop
// counter of the for loop) is unsigned. --lower-unsigned-icmp creates
// complex code at the loop header:
//
//   b1 := i>=0;
//   b2 := i<=3;
//   b3 = ite(b1, b2, true);
//   goto next;
// next:
//   assume(b3);
//
//

extern int int_nd(void);
extern char* name_nd(void);

extern void __CRAB_assume(int);
extern void __CRAB_assert(int);

typedef struct {
  char *name;
  char id;
} S1;


void foo(S1 *devices, int len) {
  int i = int_nd();
  __CRAB_assume(i >= 0);
  __CRAB_assume(i < len);
  devices[i].id = 0; 
  devices[i].name = name_nd();
}

S1 devices[4];
extern void avoid_opt(S1 *);

int main(){
  avoid_opt(&devices[0]);

  for (unsigned i=0; i<4; ++i) {
    devices[i].id = i;
    devices[i].name = name_nd();
  }

  foo(&devices[0], 4);
  
   
  int x = int_nd();
  __CRAB_assume(x >= 0);
  __CRAB_assume(x < 4);
  __CRAB_assert(devices[x].id >= 0);
  __CRAB_assert(devices[x].id <= 4);
  
  return 0;
}