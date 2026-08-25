# TeaScript

TeaScript is my high level programming language. It does not optimize for performance, but for consistency and readability. See the docs [here](https://github.com/coder-lescob/TeaScript/blob/master/doc/TeaScript.md).

## The idea

TeaScript is built around the idea that it does not matter to overcompicate things. Thus there are no class because they are too messy, instead it uses a clean and less messy system of structs and behaviors separating data from method but not by that much see the [the doc](https://github.com/coder-lescob/TeaScript/blob/master/doc/TeaScript.md) for more details.

TeaScript takes inspiration from the ease of python, the flexibility of JavaScript and several other languages including Rust. In TeaScript there are no type coercion, no implicit references.

## Interpreted ?

TeaScript is Interpreted. Because it's easier. If you are interested in implementation details, you may like the fact that TeaScript has no other dependency than libc, which comes with the C compiler. Thus to build the interpreter clone this repo and just type `make run` in a terminal (well install make if you don't have it) that's it. Note that the interpreter is still in active developement but if you are interrested in examples see [here](https://github.com/coder-lescob/TeaScript/blob/master/examples).
