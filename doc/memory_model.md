# TeaScript's memory model

## The value stack

Firstly TeaScript uses a stack to store all the variables created with let, const or static binding. The stack is a place where the variables literally stack on one another. The Arguments during a function call pass through the stack, the return values of function return through the stack.

```TeaScript
let x = 5;
let y = 6;
```

thus the stack look a bit like so:

|    Stack top       |
|--------------------|
|   Value::Int(6)    |
|   Value::Int(5)    |

```TeaScript
let f = func(x): 2*x + 1;
let a = 5;
let b = f(5);
```

|    Stack top t=0  |    Stack top (one call of f)  | Stack top (on return)  |  Stack top (after b assignment) |
|-------------------|-------------------------------|------------------------|---------------------------------|
|                   |       Value::Int(5)           |    Value::Int(11)      |         Value::Int(11)          |
|   Value::Int(5)   |       Value::Int(5)           |    Value::Int(5)       |         Value::Int(5)           |

## The variable Hashmap and lifetime encoding

As you can see the Value::Int(11) didn't move yet, it's not the return value anymore but it's the value of b. But how is the interpreter suppose to know that it's b? Well to answer that question, the interpreter assigns an unique identifier to every variable it encounters and simply has a Hashmap that maps uid to their values via what we call a ValIdx which is practically just an index in the value stack. Variables that are dead/don't exist ain't inside of the hashmap. But how does the interpreter knows when a variable dies?

Well for that a variable has a lifetime, but don't worry it's never handled manually like rust do in some places. A lifetime is literally how much a variable lives for? and for that there is another stack, this stack stores uids of variables as they are created as well as how much variables have been added in this scope in you guessed it another stack!

Once the interpreter exits from a scope it kills all the variables created in the current scope by removing them from the hashmap.

## What about a reference?

Well the value of a reference is the uid of the variable it reefers to. Any read/write are redirected (see doc for more details on the definition and use of references in TeaScript).

## Dynamic Typing

As well as it stores a ValIdx, the variable entry stores the number of members. This is used to derive how big are the values together. But it poses a problem with dynamic typing: how the heck do you fit something bigger in a smaller place of the stack?

Well the answer is give up that place, just push a new value on to the stack. But what if we store through to somewhere in a previous scope? Well the answer is either shift everything or be smart and don't do that but break the stack as you and me know it to become an arena allocator. That behaves more or less like a stack and just permits me to store new value after without killing them at the end of the scope like we do for the others!

Yes a stack for this is actually an oversimplification, but an arena allocator can behave more or less like a stack.

## What about values that vary over time in length?

Well it's time to tell you the truth, the interpreter has no real support for that. In fact that's because it supports C through ffi which can easily implement that for us. In fact it abuses the api given by the interpreter to make an allocator that is then used for the Array struct.
