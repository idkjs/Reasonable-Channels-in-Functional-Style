// Node.js Event Loop Externals
[@bs.scope "process"] [@bs.val]
external spawn : (unit => unit) => unit = "nextTick";

type can_receive
type cannot_receive
type can_send
type cannot_send

type channel('a) = {
    inputs: Queue.t((bool, 'a => unit)),
    messages: Queue.t('a)
}

// type t('a) = channel('a)  */ /* Original definition. */
type t('a, 'r, 's) = channel('a)  // Phantom type definition. */

let create = () => {
    inputs: Queue.create(),
    messages: Queue.create()
}

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

let to_read_only = channel => channel

let to_write_only = channel => channel