# Clam: Crab for Llvm Abstraction Manager #

<a href="https://github.com/seahorn/crab-llvm/actions"><img src="https://github.com/seahorn/crab-llvm/workflows/CI/badge.svg" title="Ubuntu 18.04 LTS 64bit, g++-6"/></a>


<img src="https://upload.wikimedia.org/wikipedia/en/4/4c/LLVM_Logo.svg" alt="llvm logo" width=280 height=200 /><img src="http://i.imgur.com/IDKhq5h.png" alt="crab logo" width=280 height=200 /> 

Clam is a static analyzer that computes inductive invariants for
LLVM-based languages based on
the [Crab](https://github.com/seahorn/crab) library. This branch
supports LLVM 10.

# Requirements #

Clam is written in C++ and uses heavily the Boost library. The
main requirements are:

- Modern C++ compiler supporting c++14 
- Boost >= 1.65
- GMP 
- MPFR (if `-DCRAB_USE_APRON=ON` or `-DCRAB_USE_ELINA=ON`)

In linux, you can install requirements typing the commands:

     sudo apt-get install libboost-all-dev libboost-program-options-dev
     sudo apt-get install libgmp-dev
     sudo apt-get install libmpfr-dev	

To run tests you need to install `lit` and `OutputCheck`. In Linux:

     apt-get install python-pip
     pip install lit
     pip install OutputCheck

# Installation from sources # 

The basic compilation steps are:

     mkdir build && cd build
     cmake -DCMAKE_INSTALL_PREFIX=_DIR_ ../
     cmake --build . --target crab && cmake ..
     cmake --build . --target llvm && cmake ..      
     cmake --build . --target install 


Clam provides several components that are installed via the `extra`
target. These components can be used by other projects outside of
Clam. 
  
* [sea-dsa](https://github.com/seahorn/sea-dsa): ```git clone https://github.com/seahorn/sea-dsa.git```

  `sea-dsa` is the heap analysis used to translate LLVM memory
  instructions. Details can be
  found [here](https://jorgenavas.github.io/papers/sea-dsa-SAS17.pdf)
  and [here](https://jorgenavas.github.io/papers/tea-dsa-fmcad19.pdf).
  
* [llvm-seahorn](https://github.com/seahorn/llvm-seahorn): ``` git clone https://github.com/seahorn/llvm-seahorn.git```

   `llvm-seahorn` provides specialized versions of `InstCombine` and
   `IndVarSimplify` LLVM passes as well as a LLVM pass to convert undefined values into nondeterministic calls.

The component `sea-dsa` is mandatory and `llvm-seahorn` is optional but highly
recommended. To include these external components, type instead:

     mkdir build && cd build
     cmake -DCMAKE_INSTALL_PREFIX=_DIR_ ../
     cmake --build . --target extra            
     cmake --build . --target crab && cmake ..
     cmake --build . --target llvm && cmake ..           
     cmake --build . --target install 

The Boxes/Apron/Elina domains require third-party libraries. To avoid
the burden to users who are not interested in those domains, the
installation of the libraries is optional.

- If you want to use the Boxes domain then add `-DCRAB_USE_LDD=ON` option.

- If you want to use the Apron library domains then add
  `-DCRAB_USE_APRON=ON` option.

- If you want to use the Elina library domains then add
  `-DCRAB_USE_ELINA=ON` option.

**Important:** Apron and Elina are currently not compatible so you
cannot enable `-DCRAB_USE_APRON=ON` and `-DCRAB_USE_ELINA=ON` at the same time. 

For instance, to install Clam with Boxes and Apron:

     mkdir build && cd build
     cmake -DCMAKE_INSTALL_PREFIX=_DIR_ -DCRAB_USE_LDD=ON -DCRAB_USE_APRON=ON ../
     cmake --build . --target extra                 
     cmake --build . --target crab && cmake ..
     cmake --build . --target ldd && cmake ..
     cmake --build . --target apron && cmake ..
     cmake --build . --target llvm && cmake ..                
     cmake --build . --target install 

## Checking installation ## 

To run some regression tests:

     cmake --build . --target test-simple

# Running Clam without installation #

You can get the latest binary from docker hub using the command:

     docker pull seahorn/clam-llvm10:nightly
	 
# Clam architecture #

![Clam Architecture](https://github.com/seahorn/crab-llvm/blob/master/clam_arch.jpg?raw=true "Clam Architecture")

# Example 1 #

Consider the program `test.c`:

```c
extern void __CRAB_assume (int);
extern void __CRAB_assert(int);
extern int  __CRAB_nd(void);

int main() {
  int k = __CRAB_nd();
  int n = __CRAB_nd();
  __CRAB_assume (k > 0);
  __CRAB_assume (n > 0);
  
  int x = k;
  int y = k;
  while (x < n) {
    x++;
    y++;
  }
  __CRAB_assert (x >= y);
  __CRAB_assert (x <= y);  
  return 0;
}

```

Clam provides a Python script called `clam.py`. Type the command:

    clam.py test.c

**Important:** the first thing that `clam.py` does is to compile
  the C program into LLVM bitcode by using Clang. Since Crab-llvm is
  based on LLVM 10, the version of clang must be 10 as well. 


If the above command succeeds, then the output should be something
like this:

```
Invariants for main
_1:
/**
  INVARIANTS: ({}, {})
**/
  _2 =* ;
  _3 =* ;
  _4 = (-_2 <= -1);
  zext _4:1 to _call:32;
  _6 = (-_3 <= -1);
  zext _6:1 to _call1:32;
  x.0 = _2;
  y.0 = _2;
  goto _x.0;
/**
  INVARIANTS: ({}, {_call -> [0, 1], _call1 -> [0, 1], _2-x.0<=0, y.0-x.0<=0, x.0-_2<=0, y.0-_2<=0, _2-y.0<=0, x.0-y.0<=0})
**/
_x.0:
/**
  INVARIANTS: ({}, {_call -> [0, 1], _call1 -> [0, 1], _2-x.0<=0, y.0-x.0<=0, _2-y.0<=0, x.0-y.0<=0})
**/
  goto __@bb_1,__@bb_2;
__@bb_1:
  assume (-_3+x.0 <= -1);
  goto _10;
_10:
/**
  INVARIANTS: ({}, {_call -> [0, 1], _call1 -> [0, 1], _2-x.0<=0, y.0-x.0<=0, _2-y.0<=0, x.0-y.0<=0, x.0-_3<=-1, _2-_3<=-1, y.0-_3<=-1})
**/
  _11 = x.0+1;
  _br2 = y.0+1;
  x.0 = _11;
  y.0 = _br2;
  goto _x.0;
/**
  INVARIANTS: ({}, {_call -> [0, 1], _call1 -> [0, 1], _br2-y.0<=0, _11-y.0<=0, _2-y.0<=-1, x.0-y.0<=0, x.0-_3<=0, _2-_3<=-1, y.0-_3<=0, _11-_3<=0, _br2-_3<=0, x.0-_11<=0, _2-_11<=-1, y.0-_11<=0, _br2-_11<=0, y.0-_br2<=0, _2-_br2<=-1, x.0-_br2<=0, _11-_br2<=0, _11-x.0<=0, _br2-x.0<=0, _2-x.0<=-1, y.0-x.0<=0})
**/
__@bb_2:
  assume (_3-x.0 <= 0);
  y.0.lcssa = y.0;
  x.0.lcssa = x.0;
  goto _y.0.lcssa;
_y.0.lcssa:
/**
  INVARIANTS: ({}, {_call -> [0, 1], _call1 -> [0, 1], _2-x.0<=0, y.0-x.0<=0, _3-x.0<=0, y.0.lcssa-x.0<=0, x.0.lcssa-x.0<=0, _2-y.0<=0, x.0-y.0<=0, _3-y.0<=0, y.0.lcssa-y.0<=0, x.0.lcssa-y.0<=0, y.0-y.0.lcssa<=0, _2-y.0.lcssa<=0, x.0-y.0.lcssa<=0, _3-y.0.lcssa<=0, x.0.lcssa-y.0.lcssa<=0, x.0-x.0.lcssa<=0, _2-x.0.lcssa<=0, y.0-x.0.lcssa<=0, _3-x.0.lcssa<=0, y.0.lcssa-x.0.lcssa<=0})
**/
  _14 = (y.0.lcssa-x.0.lcssa <= 0);
  zext _14:1 to _call3:32;
  assert (-_call3 <= -1);
  _16 = (-y.0.lcssa+x.0.lcssa <= 0);
  zext _16:1 to _call4:32;
  assert (-_call4 <= -1);
  @V_17 = 0;
  return @V_17;
/**
  INVARIANTS: ({_14 -> true; _16 -> true}, {_call -> [0, 1], _call1 -> [0, 1], _call3 -> [1, 1], _call4 -> [1, 1], @V_17 -> [0, 0], _2-x.0<=0, y.0-x.0<=0, _3-x.0<=0, y.0.lcssa-x.0<=0, x.0.lcssa-x.0<=0, _2-y.0<=0, x.0-y.0<=0, _3-y.0<=0, y.0.lcssa-y.0<=0, x.0.lcssa-y.0<=0, y.0-y.0.lcssa<=0, _2-y.0.lcssa<=0, x.0-y.0.lcssa<=0, _3-y.0.lcssa<=0, x.0.lcssa-y.0.lcssa<=0, x.0-x.0.lcssa<=0, _2-x.0.lcssa<=0, y.0-x.0.lcssa<=0, _3-x.0.lcssa<=0, y.0.lcssa-x.0.lcssa<=0})
**/
```

It shows the Control-Flow Graph analyzed by Crab together with the
invariants inferred for function `main` that hold at the entry and and
the exit of each basic block.

Note that Clam does not provide a translation from the basic
block identifiers and variable names to the original C program. The
reason is that Clam does not analyze C but instead the
corresponding [LLVM](http://llvm.org/) bitcode generated after
compiling the C program with [Clang](http://clang.llvm.org/). To help
users understanding the invariants Clam provides an option to
visualize the CFG of the function described in terms of the LLVM
bitcode:

    clam.py test.c --llvm-view-cfg

and you should see a screen with a similar CFG to this one:

   <img src="https://github.com/seahorn/crab-llvm/blob/dev10/demo/test.c.dot.png" alt="LLVM CFG of test.c" width=375 height=400 />

Since we are interested at the relationships between `x` and `y` after
the loop, the LLVM basic block of interest is `_y.0.lcssa` and the
variables are `x.0.lcssa` and `y.0.lcssa`, which are simply renamings
of the loop variables `x.0` and `y.0`, respectively.

With this information, we can look back at the invariants inferred by
our tool and see the linear constraints:

    x.0.lcssa-y.0.lcssa<=0, ... , y.0.lcssa-x.0.lcssa<=0

that implies the desired invariant `x.0.lcssa` = `y.0.lcssa`.


# Clam Options #


Clam analyzes programs with the `zones` domain as the default
abstract domain. Users can choose the abstract domain by typing the
option `--crab-dom=VAL`. The possible values of `VAL` are:

- `int`: intervals
- `ric`: reduced product of `int` and congruences
- `term-int`: `int` with uninterpreted functions
- `dis-int`: disjunctive intervals based on Clousot's DisInt domain
- `term-dis-int`: `dis-int` with uninterpreted functions
- `boxes`: disjunctive intervals based on LDDs (only if `-DUSE_LDD=ON`)
- `zones`: zones domain using sparse DBM in split normal form
- `oct`: Octagon domain (Apron if `-DUSE_APRON=ON` or Elina if `-DUSE_ELINA=ON`)
- `pk`:  Polyhedra domain (Apron if `-DUSE_APRON=ON` or Elina if `-DUSE_ELINA=ON`) 
- `rtz`: reduced product of `term-dis-int` with `zones`
- `w-int`: wrapped interval domain

For domains without narrowing operator (for instance `boxes`,
`dis-int`, and `pk`), you need to set the option:
	
    --crab-narrowing-iterations=N

where `N` is the number of descending iterations (e.g., `N=2`).

You may want also to set the option:
	
	--crab-widening-delay=N

where `N` is the number of fixpoint iterations before triggering
widening (e.g., `N=1`).
	   
The widening operators do not use thresholds by default. To use them,
type the option

	--crab-widening-jump-set=N

where `N` is the maximum number of thresholds.

We also provide the option `--crab-track=VAL` to indicate the level of
abstraction of the translation. The possible values of `VAL` are:

- `num`: translate only operations over LLVM registers of integer and
   boolean types.
- `sing-mem`: `num` + translate all singleton memory objects (e.g.,
   non-taken-address globals and stack variables).
- `mem`: `num` + translates all memory objects.

By default, all the analyses are run in an intra-procedural
manner. Whenever possible, we recommend to run Clam with option
`--inline`. This option will inline all function calls if the callee
is not recursive. If inlining is not desired or too expensive, enable
the option `--crab-inter` to run the inter-procedural version. Clam
implements a standard top-down inter-procedural analysis with
memoization. The analysis supports recursive functions.

Clam provides the **very experimental** option `--crab-backward`
to enable an iterative forward-backward analysis that might produce
more precise results. The backward analysis computes *necessary
preconditions* of the error states (if program is annotated with
assertions) which are used to refine the set of initial states so that
the forward analysis can refine its results.

Note that apart from inferring invariants or preconditions, Clam
allows checking for assertions. To do that, programs must be annotated
with `__CRAB_assert(c)` where `c` is any expression that evaluates to
a boolean. Note that `__CRAB_assert` must be defined as an `extern`
function so that Clang does not complain:

    extern void __CRAB_assert(int);

Then, you can type:

    clam.py test.c --crab-check=assert

and you should see something like this:

    user-defined assertion checker using SplitDBM
    2  Number of total safe checks
    0  Number of total error checks
    0  Number of total warning checks

Finally, to make easier the communication with other LLVM-based tools,
Clam can output the invariants by inserting them into the LLVM
bitcode via `verifier.assume` instructions. The option
`--crab-add-invariants=block-entry` injects the invariants that hold
at each basic block entry while option
`--crab-add-invariants=after-load` injects the invariants that hold
right after each LLVM load instruction. The option `all` injects
invariants in all above locations. To see the final LLVM bitcode just
add the option `-o out.bc`. The option `--crab-promote-assume` replaces 
`verifier.assume` instructions with `llvm.assume` intrinsics.

## Yaml inteface ##

To make easier reproducibility, you can write all the Clam options in
a `yaml` file and use the command:

    clam-yaml.py -y YCONFIG PROGRAM EXTRA
	
where `YCONFIG` is the Yaml file that contains all the options. See
directory `yaml-configurations` for examples. The command
`clam-yaml.py` simply calls `clam.py` with the options written in the
`YCONFIG` file.

# Example 2 #

Consider the next program:

```c
    extern int __CRAB_nd(void);
    int a[10];
    int main (){
       int i;
       for (i=0;i<10;i++) {
         if (__CRAB_nd ())
            a[i]=0;
         else 
            a[i]=5;
       }
       int res = a[i-1];
       return res;
    }
```

and type

    clam.py test.c --crab-track=sing-mem --crab-add-invariants=all -o test.crab.bc
    llvm-dis test.crab.bc

The content of `test.crab.bc` should be similar to:

```
    define i32 @main() #0 {
    entry:
       br label %loop.header
    loop.header:   ; preds = %loop.body, %entry
       %i.0 = phi i32 [ 0, %entry ], [ %_br2, %loop.body ]
       %crab_2 = icmp ult i32 %i.0, 11
       call void @verifier.assume(i1 %crab_2) #2
       %_br1 = icmp slt i32 %i.0, 10
       br i1 %_br1, label %loop.body, label %loop.exit
    loop.body:   ; preds = %loop.header
       call void @verifier.assume(i1 %_br1) #2
       %crab_14 = icmp ult i32 %i.0, 10
       call void @verifier.assume(i1 %crab_14) #2
       %_5 = call i32 (...)* @__CRAB_nd() #2
       %_6 = icmp eq i32 %_5, 0
       %_7 = sext i32 %i.0 to i64
       %_. = getelementptr inbounds [10 x i32]* @a, i64 0, i64 %_7
       %. = select i1 %_6, i32 5, i32 0
       store i32 %., i32* %_., align 4
       %_br2 = add nsw i32 %i.0, 1
       br label %loop.header
    loop.exit:   ; preds = %loop.header
       %_11 = add nsw i32 %i.0, -1
       %_12 = sext i32 %_11 to i64
       %_13 = getelementptr inbounds [10 x i32]* @a, i64 0, i64 %_12
       %_ret = load i32* %_13, align 4
       %crab_23 = icmp ult i32 %_ret, 6
       call void @verifier.assume(i1 %crab_23) #2
       ret i32 %_ret
    }
```

The special thing about the above LLVM bitcode is the existence of
`@verifier.assume` instructions. For instance, the instruction
`@verifier.assume(i1 %crab_2)` indicates that `%i.0` is between 0 and
10 at the loop header. Also, `@verifier.assume(i1 %crab_23)` indicates
that the result of the load instruction at block `loop.exit` is
between 0 and 5.

# Limitations #

- No support for floating point operations.

- Very little support for non-linear reasoning. Most Crab numerical
  domains reason about linear arithmetic. The `term-int` domain is an
  exception. This domain can treat non-linear arithmetic expressions
  as uninterpreted functions.

- Most Crab numerical domains reason about mathematical integers. The
  `w-int` domain is an exception because it can reason about machine
  arithmetic. The `zones` domain can use machine arithmetic but it is
  not enabled by default.

- There are several Crab numerical domains that compute some form of
  disjunctive invariants (e.g., `boxes` or `dis-int`) but they are
  still limited in terms of expressiveness to keep them tractable.

- The backward analysis is too experimental and it requires more work.

- Reasoning about memory relies heavily on `sea-dsa` which is context-
  and field-sensitive pointer analysis but flow-insensitive.
  
  
