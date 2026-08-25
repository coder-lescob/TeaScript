# TeaScript

TeaScript is my high-level interpreted programming language. It's licensed under GPL v3. TeaScript is the result of me poking around and programming in various programming languages: C, C++, Rust, Python and even JavaScript (even tho I didn't do a lot of it). I liked some concepts of all these languages but they always had some less good features e.g. coercions in JavaScript, too much strictness, while also restraining the freedom in Rust, not enough garentee in C/C++. All theses made me come to the logical conclusion that altho there are some good idea, none of them is perfect. So since none is perfect, let's create the perfect one. I should specify that with TeaScript I'm actually doing the high-level part of the spectrum, the low-level will be with Steel.

## The promise

TeaScript aimes to make programming easier for programmers, while not restricting them nor make them obey nonsense (e.g. long ceremony syntax, inheritances, coercion rules, ...).

## The language

### Types

TeaScript is dynamically typed, because it is convienent however with dynamic typing there is often a caveat: the programmer may have not control over the type. So it's why TeaScript has a specific feature that make that possible: type locking. Type locking is simple if you don't want a variable to be of any other type than a specified one it can be specified with type annotations.

### Variable declarations

Variables are declared with the binding `let`. like so:

```TeaScript
let x = 5;
```

As mention before TeaScript implements type locking which can be used to lock a type with a type annotation:

```TeaScript
let x: Int = 5; // the interpreter garentee that x shall at all times be an integer
```

Type locking can infer the type from the value, with the type auto:

```TeaScript
let x: auto = 5; // the interpreter lock x to be an Int, infered from the type
```

This behavior is to avoid repeating long types and is not recomended for most data and is only a feature because you'll see there'll be generic type.

Variables can be declared constants with the binding `const`:

```TeaScript
const x = 5;
// note that shadowing a constant is forbidden and these two declarations shall not be thought to belong to the same script
const x: Int = 5; // both work but annotation may be a bit useless in some cases
```

Constants variables are immutable in all sense of the term, the object can't mutate nor can the binding, unlike rust variables that are immutables by default in TeaScript a varibale is mutable, but a constant is immutable because that's the meaning of the word constant and variable imply that the value vary.

In TeaScript a static variable is one that can be mutated but it's binding is immutatble. e.g.

```TeaScript
static x = 5;
x++; // x = 6
x = 5; // not allowed. obviously it's stupid for integers.
```

### Variables uses

Variables can be read by using the variable name.

The `=` symbol is used to assign a new value to a variable, an assignement always copies entirely the right hand side:

```TeaScript
let x = 5;
x = 6; // now x=6
```

#### Operators

TeaScript support a lot of operator like addition with `+`, subtraction with `-`, multiplication with `*` and divison with `/`. Here is the table of operators:

|operators|description                                                                                           |type           |kind of arithmetic|
|---------|------------------------------------------------------------------------------------------------------|---------------|------------------|
| `+`     | adds two values                                                                                      |binary operator|normal            |
| `-`     | subtract a value from another                                                                        |binary operator|normal            |
| `*`     | multiply two values together                                                                         |binary operator|normal            |
| `/`     | divide a value by another                                                                            |binary operator|normal            |
| `&`     | perform a bitwise and operation on two values every bit on in both numbers will be on in the result  |binary operator|bitwise boolean   |
| `\|`    | perform a bitwise or operation on two values every bit on in either of them will be on in the result |binary operator|bitwise boolean   |
| `^`     | perform a bitwise exclusive or on two values every bit on in one but not the other will be on        |binary operator|bitwise boolean   |
| `!`     | inverts all the bits from a value                                                                    |prefix operator|bitwise boolean   |
| `<<`    | shifts all the bits of a value to the left                                                           |binary operator|shifting          |
| `>>`    | shifts all the bits of a value to the right                                                          |binary operator|shifting          |
| `&&`    | boolean and: tells if two conditions are true                                                        |binary operator|boolean           |
| `\|\|`  | boolean or: tells if either of two conditions are true                                               |binary operator|boolean           |
| `^^`    | boolean xor: tells if either of two conditions are true while the other isn't                        |binary operator|boolean           |

binary operators act on two values, prefix operators process a single value and is marked in front of it.

#### Expressions

An expression is a combinasion of values and operator that yeilds a result, getting the result is called evaluating an expression. example:

```Expression
5.0*4.0-8.0/(5.0+1.0) evaluates to 18.666666667
```

In an expression parentheses indicate priority explicitly. In an expression when the name of a variable is hit, the right value for that variable is substitued. For a normal variable the right value is it's value tho for others it might not be (especially for references).

#### Sugar

So in TeaScript a variable is declared with an expression that is then evaluated to know the value of the variable. When assigning to variable, an expression is once again used. Since adding to a variable an expression, subtracting to a variable an expression... are commum pattern the language presents syntatic sugar for then in the form of `+=`, `-=`...

Another commun pattern is to increment/decrement a variable's value so the language provide syntatic sugar for them too with `++` and `--`, note that unlike C/C++ they ain't expressions.

example:

```TeaScrpt
let x = 5;
x += 2; // x = 7
x--;    // x = 6
x = x * 2 + 1; // x = 13
```

### References

In TeaScript references are explicit without needing to turn in a ceremony nightmare.

#### Definition

In TeaScript a reference has a simple definition: ***A reference is a path towards a value, any read/writes follow this path***. Anything that obeys this definition is a reference.

#### Usage

In TeaScript to take the reference of a variable use the `&` symbol in front.

```TeaScript
let x = 5; // variable
let y = &x; // reference to x
y = 6; // now x = 6, 6 is routed towards x and not y
```

If you need to change the binding of y shadow it, shadowing a variable creates a new variable with the same name, thus as long as the shadow is alive the shadowed variable cannot be used, but it's still alive. e.g.

```TeaScript
let x = 5; // variable
let y = &x; // reference to x
y = 6; // now x = 6, 6 is routed towards x and not y

let y = 5; // y = 5 and isn't a reference anymore.
```

bindings are bound to the current scope e.g.

```TeaScript
let x = 5;
let y = &x;

y = 6; // x = 6

{
    let y = 5; // y = 5, x = 6
    y++; // y = 6, x = 6
}

y = 89; // x = 89

```

#### Nested References

As you saw references in TeaScript behave more as aliases than pointers. And that's intentionnal. Not that pointers are bad, they give you tons of controls, just that pointers ain't high level but belong more to lower level languages.

In TeaScript nested references don't exists because a path to a path is always a path. However some behavior can be very similar:

```TeaScript
let x = 5;
let y = &x;
let z = &y; // here y is read as x thus z == &x
```

The path followed by each reference can be thought as nested but really isn't. It isn't:

```Diagram
z -> y -> x
```

but:

```Diagram
z -+---> x
y -|
```

### Control flow

TeaScript include multiple control flow mechanics such as if/else statements and pattern matching.

#### If/else

The syntax is deceptivally simple:

```TeaScript
if condition {
    // code executed if condition met
}
else { // else optional
    // code executed otherwise
}
```

If statements also include `else if` statements:

```TeaScript
if condition {
    // code executed if condition met
}
else if other_condition { // optional
    // code executed otherwise if the other_condition is met
}
else { // optional
    // code executed otherwise if none of the others conditions are met.
}
```

In TeaScript a condition is not truthy: **1 != true**.

#### Pattern matching

TeaScript provide advenced pattern matching. It includes guards and destructuring. To delare a pattern matching use the keyword `switch` followed by what you want to match. Any matching arm starts with a pattern to match for followed by `=>`. The pattern may be a value (e.g. 1, "hello"), a guards (e.g. x if x > 0) or a destructure (e.g. SomeStruct { field1: value, ... }).

```TeaScript
switch value {
    0 => println("zero"),
    1 => println("one"),
    x if x < 0 => println("negative"),
}
```

The pattern matching does not require exhaustiveness (see the error section for more informations).

#### Loops

##### While

A while loop loops while a condition is true, the condition is checked at the start of the iteration. e.g.

```TeaScript
while condition {
    // code executed while the condition is true
}
```

##### Do While

A do while loop is the same as a while loop but it checks the condition at the end of the iteration.

```TeaScript
do {
    // code executed while condition is true
}
while condition;
```

##### For

There are two types of for loops. There are the classic one and the modern one. The classic one goes like so:

```TeaScript
for (let i = 0; i < 50; i++) {
    // code repeated while i < 50, i starting at 0 and advencing of 1 each iteration
    // the first thing has to be a statement and the last too like so:
    // for (statement_init; condition; statement_at_iter) { /* code */ }
}
```

And the modern one loops over all the objects of a container, it goes like so:

```TeaScript
for (let obj in objs) {
    // code ran for every object in objs
}
```

All control flow can be used as statements or as expressions.

```TeaScript
let x = if y == 1 { 32 } else { 27 };
```

### Functions

#### Definition

A function is callable block of code. It has parameters, which permit the caller to pass data in the callee. It also has a returned value, this value permits the function to return a value to the caller.

#### Declaration

A function is declared using the keyword `func`, followed by the name of the function then the parameters in parentheses. The return type can be put after using `-> type`, but it's not mendatory, like with variable declaration it garentees that the returned value has the type. Likewise, parameters can have types indicated using `: type` after the parameter name.

```TeaScript
func function_name(arg1, arg2) -> ret {
    // code
}
```

#### Return

To return a value from a function use `return value;`. But there is syntactic sugar in the case of just a function returning a value. Use `: value;` as sugar for `{ return value; }`.

#### Closure

Functions are first class values. Therefore they can be stored, access and passed around. thus:

```TeaScript
let f = func(x): x * x;
let y = f(5); // y = 25
```

### Structs and Behaviors

In TeaScript there is not class nor inheritance. But there are `struct` and `behavior`, a `struct` can have an implementation in the `impl` block, a `behavior` is a set of functions that can be implemented by a `struct`. e.g.

```TeaScript
// a set of function
behavior Greet {
    func greet(self: Self);
}

// a structure
struct Person {
    name: String;
    age: Int;
}
impl { // the impl block shall be glued to the struct definition, may not be implemented. 
    func new(name: String, age: Int) -> Self {
        return Self { name: name, age: age }; // structure construction
    }
}

// may not be glued to the struct definition
impl Greet for Person {
    func greet(self: Self) {
        println($"hello {self.name} !");
    }
}
```

methods may modify the struct state, these functions takes a first parameter of type `&Self` reference to a `Self`, you may also create methods that don't modify the struct state, those take the first parameter of type `Self`.

```TeaScript
struct Person {
    name: String;
    age: Int;
}
impl {
    func new(name: String, age: Int) -> Self {
        return Self { name: name, age: age };
    }

    /// modifies the person
    func birthday(self: &Self) {
        self.age++; // at your birthday your age increments
    }

    /// does not modify the person
    func ask_for_pen(self: Self, who_is_asking: Self) -> Bool {
        return who_is_asking.age >= 5; // 5 years old would break your pen.
    }
}
```

#### Delegation

Use `delegate Self::field_to_delegate;` to automatically say: "if there is not this function, try call this field for the same function".

### Enums

There are two kinds of enum in TeaScript, enum and struct enum.

#### Enum

An enum is an enumeration. It's a shorthand to create a list constant. Fist index starts at but can be changed.

```TeaScript
enum {
    A, // A == 0
    B, // B == 1
    /* ... */
}
```

```TeaScript
enum {
    A = 10, // A == 10
    B, // B == 11
    /* ... */
}
```

#### struct enum

An enum struct carries data with. It's shorthand to create structures related together by some semantic meaning, like in rust.

```TeaScript
// example for fictional web server
struct enum MyRequests {
    Authenticate(String, String),
    CreateAccount(String, String),
    Logout(String),
    /* ... */
}
```

### Error handling

In TeaScript there is no exception. Errors are first class values and are returned through the normal return flow of functions. Any type, even if locked, is either the locked/current type or an Err value. More precisly the type locking garentees what the successful type MUST be, tho it does no say anything about the Err type.

```TeaScript
let x: Int = 10 / 0; // now x == Err<DivisionByZero>(10)
```

#### ?

The `?` control flow says: "if there is an error make the current function return it otherwise just return the value". for example:

```TeaScript
func not_cool() {
    let x = failible()?; // here x is never printed because the function returned before.

    println(x);
}
```

When the error propagates out of the main function the Error is shown.

#### default

The default operator provide a default value if its left side is an error.

```TeaScript
let x = 10 / 0 default 0; // here 10 / 0 == Err<DivisionByZero>(10) thus default provide a default and x == 0
```

examples of commun error

#### pattern matching exhaustiveness

When a branch is not matched, if it's a statement it does nothing and if it is an expression it returns `Err<NoBranchFor>(value)`.

#### truthyness violation

When there is a truthyness violation (aka `if 1 {}`), if the `if` is used as a statement then the current function returns `Err<TruthynessViolation>(1)` like if I had put a `?`, however when the if is used as an expression, the propagation shall be explicit and if there is no `?` the variable whose value is assigned the expression, the new value of the variable is `Err<TruthynessViolation>(1)`. Furthermore if the if is in an expression and that the condition isn't met, if no branch of the if has its condition met, and there is no `else` then it returns `Err<NoElse>()`.

#### Constant violation

If someone tries to modify a constant, the current function returns `Err<ConstViolation>("constant_name")`.

#### Type violation

If someone tries to assign or modify a value of a variable such that the locked type isn't the type of the expression and that the type isn't an error, the new value of the variable is therefore `Err<TypeViolation>("locked_var")`.

#### Binding Violation

If someone tries to modify the binding of an `static` variable, the current function return `Err<BindingViolation>("static_var")`.

#### Function call

When a function does not exist it returns `Err<NotImplemented>("function_name")`, and if it has no meaningful way of storing that error the current function returns it.

```TeaScript
func oopsy() {
    let x = does_not_exist(); // x == Err<NotImplemented>("does_not_exist")
    return x;
}
```

#### Generally

More generally if an error has a meaning to be somewhere then it is else it's returned by the current function.

#### User defined errors

You can define your own error type by creating a struct. Note that this struct must implement the behavior `ErrDisplay` which will modify how the error is shown to the user of your program.

example:

```TeaScript
use std;

struct StringMustFinishWithS {
    str: String;
}
impl {
    func new(str: String) -> Self {
        return Self { str: str };
    }
}

impl ErrDisplay for StringMustFinishWithS {
    func get_error_str(self: Self) -> String {
        return $"expected string {self.str} to finish with an 's'";
    }
}

func main() {
    let user_str = get_user_input(question: "Give a string ending in 's'");

    if !user_str.end_with('s') {
        return Err<StringMustFinishWithS>(user_str);
    }

    println($"success {user_str}, finishes with an 's'");
}
```
