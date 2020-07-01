# [Reasonable Channels in Functional Style](https://medium.com/@cleverti/reasonable-channels-in-functional-style-1ae7d71f41db)

Why your next Node.js project should be written in a functional language: a case study in static capabilities using ReasonML.

![](https://cdn-images-1.medium.com/max/2000/1*z1kKV2z0MySKHzxomL3Jug.jpeg)

I’ve long been a fan of [OCaml](https://ocaml.org/), so when I discovered [BuckleScript](https://bucklescript.github.io/) and [ReasonML](https://reasonml.github.io/) I naturally became very excited :) Why? Because these tools allow to re-target the battle-tested OCaml compiler to generate readable JavaScript that can run on Node.js and in the browser.

In my opinion, OCaml and other functional languages with type inference will begin to engage more and more programmers in the near future. The reason is that these languages maintain a concise style without losing the many benefits of static typing and of a sound type system. For example, OCaml supports parametric polymorphism — the so-called generics — since 1996 and in most cases without any type annotation! It also has a powerful module system, facilitating the development of large programs.

If you don’t know much about functional languages, and OCaml in particular, I recommend you to read [OCaml for the Masses](https://queue.acm.org/detail.cfm?id=2038036) by Yaron Minsky of Jane Street Capital. For in-depth learning, I highly recommend the book [Real World OCaml](https://dev.realworldocaml.org/) which you can read for free online.

ReasonML adapts a bit the syntax for OCaml, to make the language more natural for people with a JavaScript background. And the community is taking notice: ReasonML received the *Prediction Award* in the [2018 State of JavaScript survey](https://2018.stateofjs.com/awards/). It has also been [reported](https://reasonml.github.io/blog/2017/09/08/messenger-50-reason) that Facebook, ReasonML’s creator, has already migrated 50 percent of its messenger web application to ReasonML using [bindings](https://github.com/reasonml/reason-react) for the popular React library. ReasonML has first class support for JSX, which makes the React integration very natural.

In this article we will focus on the back-end, and specifically on ReasonML as a language for developing Node.js applications without having to suffer writing programs in JavaScript. Of course, the ecosystem is still immature, and some interfacing with libraries written in JavaScript will usually be needed. Fortunately, BuckleScript provides a well designed foreign-function interface (FFI) and with a bit of work, things work pretty well.

Now, you might wonder, why not just use OCaml directly if we are to remain in the back-end? After all, natively compiled OCaml runs very fast, and it has good event loop implementations. In fact, it is also possible to compile OCaml bytecode to JavaScript, using [js_of_ocaml](https://ocsigen.org/js_of_ocaml/), so this is another way to use OCaml for full-stack development. This method is more robust as it allows to use typed OCaml libraries everywhere in the front-end, instead of resorting to a foreign function interface against untyped code. On the other hand, this approach does not really facilitate a gradual transition for existing JavaScript codebases, it does not support JSX and React, and so it may not be preferable for most projects. See this [article](https://medium.com/@javierwchavarri/adopting-reason-strategies-dual-sources-of-truth-and-why-gentype-is-a-big-deal-c514265b466d) for further discussion on migrating to ReasonML.

## Pre-requisites

If you want to experiment with the code, please make sure you have a relatively recent version of Node installed. I’ve tested everything with 8+ and it works fine. We use a very small surface of the Node API anyway.

For BuckleScript you have two options: install globally or locally just for this project. For a global installation open a terminal, then type npm install -g bs-platform. As an alternative, you can just create a fresh directory and go for npm install --save-dev bs-platform to install locally.

Now go to your chosen project directory and type bsb -init channels -theme basic-reason in a terminal. This will bootstrap a project in a new channels directory, with reasonable defaults that work out of the box. All the code we will present should go to the src/ sub-directory.

*All source code presented in this article is covered by the [MIT license](https://opensource.org/licenses/MIT).*

## Static Capabilities using Phantom Types

The purpose of this article is to demonstrate a powerful facet of the OCaml type system: the ability to encode and enforce certain *capabilities* as types. The compiler then ensures that these capabilities are respected everywhere in our program. In other words, we get capabilities for free, without any runtime cost.

The technique we will use is based on [phantom types](https://wiki.haskell.org/Phantom_type), used frequently to encode static access control; see this [post](https://blog.janestreet.com/howto-static-access-control-using-phantom-types/) for a good introduction. For an in-depth understanding of static capabilities, see the article [Lightweight Static Capabilities](http://okmij.org/ftp/papers/lightweight-static-capabilities.pdf). Note that this technique can be applied in most languages with parametric polymorphism; see for example [Phantom Types in Scala](https://blog.codecentric.de/en/2016/02/phantom-types-scala/).

To illustrate the concepts, we are going to break things down in three stages:

* We show the interface of a Channel module, without any capabilities;

* We perform the necessary modifications to enable a much finer control of how each channel can be used;

* We finally provide an implementation of the interface.

## Basic channels

We begin by presenting the *interface* of a reasonable channel abstraction in ReasonML. In this context, by interface we mean the type of a *module*, which is the basic abstraction mechanism in OCaml/ReasonML. A module, in turn, can be thought of as a collection of types and values. Each file defines a module (or interface) of the same name.

In our case:
> *The file Channel.re will become a module Channel; Channel.rei will constrain the externally visible interface of Channel, inside any module that imports it using open Channel. Note that *inside* the file Channel.re the compiler will continue to see the unconstrained, most general types.*

Here is our first attempt, Channel.rei, which should be placed in the src/ folder.

    /* Channel.rei */

    type t('a)
    /* Internal representation of Channel type, polymorphic on type variable 'a. */

    let create: unit => t('a)
    /* Create a new channel that carries values of type 'a. */

    let send: t('a) => 'a => t('a)
    /* Send a value of type 'a on a channel of type t('a) */

    let recv: t('a) => ('a => unit) => t('a)
    /* Receive a value of type 'a, handled by a function of type 'a => unit. */

    let listen: t('a) => ('a => unit) => unit
    /* As above, but continue to listen for messages. */

    let recv_sync: t('a) => option('a)
    /* Receive if there is a value, synchronously. */

Some comments are in order. First, the type t('a) is *abstract*. We do not know how it will be implemented, and we have no way to create or manipulate it outside of the interface. This abstraction boundary is the primary method of encapsulation afforded by modules in OCaml/ReasonML. The part 'a is a type variable, similar to the ones found in other languages with parametric polymorphism, such as Java. After we create a channel, this type variable can be instantiated to any type.

Let us give an example, assuming that we have some implementation of the above interface in a file named Channel.re. We will implement this module shortly, but we don't care about the details right now, because we want to program *against an interface*.
```re
    /* File Example.re; this is a comment. */

    open Channel
    /* Make available all the visible definitions of module Channel. */

    let chan = create()
    /* Apply Channel.create to a unit argument (), and
       return a value of type t('a) which will be thereafter
       available as chan. The type 'a is not yet instantiated. */

    send(chan, 5)
    /* Apply Channel.send to chan of type t('a) and 5 of type int,
       with the intended semantics of sending 5.
       The type unification algorithm will introduce the equation
       'a=int to satisfy the signature of send, and the channel
       now has type t(int). */

    recv(chan, x => Js.log(x + 5))
    /* Receive a value from chan and use it to print on the console;
       because of the send above, x will be constrained to have the type
       int, since chan has type t(int). The callback x => Js.log(x + 5)
       has type int => unit, which matches 'a => unit in the interface. */
```

Observe that values returned by Channel.create are *polymorphic*: we can create one and use it to communicate any type of message as long as we do so consistently. So, if we add send(chan, "Hello") above, type-checking fails because the type string is not compatible with t(int). Also, we did not write any types in Example.re: the compiler can *infer* them in a way that guarantees that they are as general as possible. By extension, there is no requirement to have an interface for a module, unless if we wish to control what is exposed.

Most functions in our interface return a channel of type t('a), which allows to chain operations. This is very useful but can also get quite confusing: imagine having to write send(send(chan, 4), 5) to send first 4 and then 5. As an alternative we can follow a more verbose style and write let _ = send(chan, 4); let _ = send(chan, 5), which is slightly better but abandons chaining altogether. (As usual, _ is a variable name commonly used for ignored values.)

Here are two ways to make things more readable:

* We can use the standard reverse-application operator |> which allows to write x |> f |> g for g(f(x)). BuckleScript additionally allows to put a placeholder _ and direct the parameter to a specific position, so we can write chan |> send(_, 5) |> send(_, 6). BuckleScript converts send(_, 5) to x => send(x, 5).

* We can use the [fast pipe](https://bucklescript.github.io/docs/en/fast-pipe) operator |. (or more commonly -> in ReasonML). This is designed for the above use case, injecting the piped value *before* other parameters. Concretely, chan |. send(5) and chan -> send(5) are equivalent to send(chan, 5). We can now write chan |. send(5) |. send(6) which is exactly what we want. The operator is left associative which means that the previous example is interpreted as (chan |. send(4)) |. send(5).

We now return to the last two functions declared in the interface. The first, listen, means to do recv forever instead of just once. The second, recv_sync, will allow to immediately receive a value if there is one. It's the *synchronous* version of recv. In that case there is no callback given, and the intended usage is let msg = recv_sync(chan). The built-in type option('a) indicates that msg will either be a value Some(m) of type Some('a), where m is of type 'a, or it will be None with type None. This kind of type is called [variant](https://reasonml.github.io/docs/en/variant).

Here’s how recv_sync would be normally used:

```re
    let msg = recv_sync(chan);
    switch(msg) {
        Some(m) => ... /* Do something with m. */
      | None => ... /* Handle the no message case. */
    }
```

We are now ready to improve the Channel interface with capabilities.

## Permissioned channels

Here is the new interface Channel.rei:
```re
    /* Channel.rei */

    type t('a, 'r, 's)
    /* Internal representation of Channel type.
       Type parameters:
       'a: type of value transmitted.
       'r: receive permission.
       's: send permission. */

    type can_receive
    type cannot_receive
    type can_send
    type cannot_send

    let create: unit => t('a, can_receive, can_send)
    /* Or: => t('a, 'r, 's) */

    let send: t('a, 'r, can_send) => 'a => t('a, cannot_receive, can_send)
    /* Or: => t('a, 'r, can_send) */

    let recv: t('a, can_receive, 's) => ('a => unit) => t('a, can_receive, cannot_send)
    /* Or: => t('a, can_receive, 's) */

    let listen: t('a, can_receive, 's) => ('a => unit) => unit
    /* Or: => t('a, can_receive, 's) */

    let recv_sync: t('a, can_receive, 's) => option('a)

    let to_read_only: t('a, can_receive, 's) => t('a, can_receive, cannot_send)
    /* Remove the send capability. */

    let to_write_only: t('a, 'r, can_send) => t('a, cannot_receive, can_send)
    /* Remove the receive capability. */
```

Let’s look at the key points one by one:

* We added two more type variables to the type t, which is now t('a, 'r, 's). The new variables are placeholders for permissions to *receive* ('r) and *send* ('s).

* We defined 4 new types, which are going to be used as permissions: can_receive, cannot_receive, can_send, and cannot_send. The intended usage is to instantiate 'r with can_receive or cannot_receive and 's with can_send or cannot_send. (Emphasis on *intended*, but let's avoid too much detail.)

* In the comments, we show alternative result types that we could define and the compiler would still be happy. As a small example, chan |. send(5) |. recv(x => ...) does *not* type check even if it would be perfectly safe because it can obscure the fact that 5 will be immediately received. On the other hand, let _ = chan |. send(5); let _ = chan |. recv(x => ...) is fine.

* We added functions to obtain receive-only and send-only handles to a channel. This is reflected in the changing permissions.

Time for an example:

```re
    /* Example1.re */

    open Channel

    let chan = create()  /* chan: t('a, can_receive, can_send) */

    let chan_ro = to_read_only(chan)  /* chan_ro: t('a, can_receive, cannot_send) */

    chan_ro |. send(5)  /* Rejected by the compiler. */
```

At the end of the article, we are going to see exactly how the compiler will respond to the above code.

## Implementing the interface in module Channel.re

We begin by using the [FFI](https://bucklescript.github.io/bucklescript/Manual.html#_binding_to_simple_js_functions_values) to import the process.nextTick function as spawn. (If we wanted a more portable solution, we could use Promises which also work on modern browsers.)
```re
    /* Channel.re part 1 of 7 */

    /* Node.js Event Loop Externals */
    [@bs.scope "process"] [@bs.val]
    external spawn : (unit => unit) => unit = "nextTick";
```

In the next fragment, we define the types. Note that the send-receive capabilities remain *abstract*: we must provide them because they appear in Channel.rei, but we are not required to actually define them concretely. Next, we define a polymorphic record type for channels. This provides access to two queues, one for the pending input callbacks and one for the pending messages. More accurately, the input queue will contain tuples of (boolean, callback): if the boolean is true, then the input is a server (it repeats); if it is false, the input can only be performed once (or zero times). Finally, the type t which we are again obligated to define is identified with channel. Since the variables 'r and 's do not appear in channel('a), it follows that for *any* types a, r, s the compiler knows that t(a, r, s) == channel(a). The permissions are therefore irrelevant *inside* the Channel module.

```re
    /* Channel.re part 2 of 7 */

    type can_receive
    type cannot_receive
    type can_send
    type cannot_send

    type channel('a) = {
        inputs: Queue.t((bool, 'a => unit)),
        messages: Queue.t('a)
    }

    /* type t('a) = channel('a)  */ /* Original definition. */
    type t('a, 'r, 's) = channel('a)  /* Phantom type definition. */
```

From now on we don’t need any type annotations, and in fact the code is the same as the one we would have written for the simple channel interface without permissions. First, we define create that returns a fresh channel, i.e., a record. OCaml will find the definition channel('a) and infer that it can assign it to the returned value. As a result, inside the module create has type unit => channel('a). However, outside of the module it has the return type declared in the interface, i.e., it is assigned the type unit => Channel.t('a, can_receive, can_send).

```re
    /* Channel.re part 3 of 7 */

    let create = () => {
        inputs: Queue.create(),
        messages: Queue.create()
    }
```

We now implement the communications. Here I’m adapting what is known as [Turner’s Abstract Machine](http://www.lfcs.inf.ed.ac.uk/reports/96/ECS-LFCS-96-345/). It’s a way to implement the [pi-calculus](https://en.wikipedia.org/wiki/%CE%A0-calculus), which can be understood as the lambda-calculus of concurrency. But let’s leave theory for another day. One detail worth mentioning is the rec part: to define mutually recursive functions, we write let rec f1 = ... and f2 = ... and fn = ... to help the type checker.

The main idea of send is as follows: if there is some input waiting to receive on the same channel, perform the communication; else, put the message in the channel queue *until* an input appears to take it.

```re
    /* Channel.re part 4 of 7 */

    let rec
    send = (channel, msg) => {
        let {inputs, messages} = channel
        if (Queue.length(inputs) > 0) {
            let (is_replicated, receiver) = Queue.take(inputs)
            channel |> communicate(_, msg, receiver, is_replicated)
        } else {
            Queue.push(msg, messages)
        }
        channel
    }
    and
```

Next, we implement recv' which is the basis for both recv and listen. It works dually to send: if there is a message, take it; else put the callback in the inputs queue so that eventually it can match some output.

```re
    /* Channel.re part 5 of 7 */

    recv' = (channel, receiver, is_replicated) => {
        let {inputs, messages} = channel
        if (Queue.length(messages) > 0) {
            let msg = Queue.take(messages)
            channel |> communicate(_, msg, receiver, is_replicated)
        } else {
            let input = (is_replicated, receiver)
            Queue.push(input, inputs)
        }
        channel
    }
    and
```

When a communication can take place, which means we have both a message and a receiver callback on a channel, communicate is called. We are relying on spawn (i.e., process.nextTick) to delay the execution, as is usual in control flow for concurrency primitives; Promises also do something along these lines. There is, of course, a more fundamental reason: if we run the callback with the message immediately, we can *starve* the event loop.

In the case of listen, the value of is_replicated will be true, so we will also spawn the input again. (Observe that communicate and recv' do not appear in Channel.rei, so they are *private* to the module.)

As for run_safe, it catches any exceptions thrown in the callback for a message and prints them; normally we could put better mechanics here, but it's not important for now. See the [docs](https://reasonml.github.io/docs/en/exception) for details on exception handling.

```re
    /* Channel.re part 6 of 7 */

    communicate = (channel, message, receiver, is_replicated) => {
        spawn( () => run_safe(receiver, message) )
        if (is_replicated)
            spawn( () => recv'(channel, receiver, is_replicated) |. _ => () )
    }
    and
    run_safe = receiver => message => {
        try (receiver(message)) {
            | Js.Exn.Error(e) => Js.log({j|JS Error: $e|j})
            | e => Js.log({j|Error: $e|j})
        }
    }

    let recv = (channel, receiver) => recv'(channel, receiver, false)

    let listen = (channel, receiver) => recv'(channel, receiver, true) |. _ => ()

    let recv_sync = channel => {
        let {messages} = channel
        Queue.length(messages) > 0 ? Some(Queue.take(messages)) : None
    }
```

Finally, we implement the conversions to restrict capabilities. It’s as simple as it gets, even if at first sight it makes no sense. How is it possible that the identity function has different argument and return types? The trick is that permissioned types are *identified* with the permission-less type channel inside the module, and are therefore considered equal. For example, the compiler needs to prove that to_read_only can be assigned the type t('a, can_receive, 's) => t('a, can_receive, cannot_send), which inside the module is equivalent to proving that it has the type channel('a) => channel('a).

```re
    /* Channel.re part 7 of 7 */

    let to_read_only = channel => channel

    let to_write_only = channel => channel
```

Ready to run some examples? Create a file Example.re, open the channel module, and go for it! The workflow consists in running npm run build then node src/Example.bs.js to run the JS code generated from your Example.re module. It's also a good idea to study the generated *.bs.js files, they are very readable.

Here’s something to get you started:

```re
    /* Example2.re */

    open Channel

    let end' = _ => ()  /* = ignore from Pervasives */

    let filter = (source, pred) => {
        let target = create()
        source |. listen( m =>
            pred(m) ? target |. send(m) |. end' : ()
        )
        to_read_only(target)
    }

    let chan = create()

    chan |. filter(i => i > 0)
         |. listen(i => Js.log(i))
         |. end'

    chan |. send(1)
         |. send(-1)
         |. end'

    chan |. send(2) |. end'

    /* Output:
       1
       2
    */
```

To run the above, first do npm run build and then node src/Example2.bs.js.

Now try the same with Example1.re and the compiler will immediately inform you that you have a bug! The output should be something along these lines:

```re
    9 │ chan_ro |. send(5)  /* Rejected by the compiler. */
```
```sh
    This has type:
    Channels.Channel.t('a, Channels.Channel.can_receive,
                           Channels.Channel.cannot_send)
        ...
    But somewhere wanted:
    Channels.Channel.t('a, Channels.Channel.can_receive,
                           Channels.Channel.can_send)
        ...

    The incompatible parts:
    Channels.Channel.cannot_send
    vs
    Channels.Channel.can_send
```

Pretty self-explanatory, right?

## Conclusion

The OCaml compiler does a lot of heavy lifting for us in finding and also in explaining the errors. This helps to eliminate a large class of potential bugs and to exercise fine-grained control over our abstractions. Still, types don’t really get in the way: we can mostly avoid to write them, resulting in clean and readable code.

In a future article, we will present a complete set of utilities for reactive/stream programming, using Channel as the core building block. This new module can subsume much of the functionality found in frameworks like [RxJS](https://rxjs-dev.firebaseapp.com/). In fact, it achieves better static guarantees with an implementation that is an order of magnitude smaller. Stay tuned!

*Written by Dimitris Mostrous | Tech Lead at Cleverti*

*Originally published at [www.cleverti.com](https://www.cleverti.com/news/reasonable-channels-in-functional-style).*
