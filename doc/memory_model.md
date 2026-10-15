# TeaScript's memory model

## The stack

Firstly TeaScript uses a stack to store all the variables created with let, const or static binding. The stack is a place where the variables literally stack on one another. The Arguments during a function call pass through the stack, the return values of function return through the stack.

```TeaScript
let x = 5;
let y = 6;
```

thus the stack look a bit like so:

|    Stack top       |
|--------------------|
|      Value(6)      |
|      Value(5)      |

```TeaScript
let f = func(x): 2*x + 1;
let a = 5;
let b = f(5);
```

|    Stack top t=0  |    Stack top (one call of f)  |  Stack top | Stack top  |  Stack top (on return)  |  Stack top (after y assignment) |
|-------------------|-------------------------------|------------|------------|-------------------------|---------------------------------|
|                   |                               | Value(10)  | Value(11)  |                         |                                 |
|                   |          Value(5)             | Value(5)   | Value(5)   |      Value(11)          |           Value(11)             |
|      Value(5)     |          Value(5)             | Value(5)   | Value(5)   |      Value(5)           |           Value(5)              |
