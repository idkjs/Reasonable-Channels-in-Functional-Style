// Internal representation of Channel type.
//  Type parameters:
//  'a: type of value transmitted.
//  'r: receive permission.
//  's: send permission.
type t<'a, 'r, 's>

type can_receive
type cannot_receive
type can_send
type cannot_send

let create: unit => t<'a, can_receive, can_send>
// Or: => t<'a, 'r, 's> */

let send: t<'a, 'r, can_send> => 'a => t<'a, cannot_receive, can_send>
// Or: => t<'a, 'r, can_send> */

let recv: t<'a, can_receive, 's> => ('a => unit) => t<'a, can_receive, cannot_send>
// Or: => t<'a, can_receive, 's) */

let listen: t<'a, can_receive, 's> => ('a => unit) => unit
// Or: => t<'a, can_receive, 's) */

let recv_sync: t<'a, can_receive, 's> => option<'a>

let to_read_only: t<'a, can_receive, 's> => t<'a, can_receive, cannot_send>

let to_write_only: t<'a, 'r, can_send> => t<'a, cannot_receive, can_send>
