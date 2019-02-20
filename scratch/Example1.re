// open Channel
// // chan: t('a, can_receive, can_send)
// let chan = create()
// //chan_ro: t('a, can_receive, cannot_send)
// let chan_ro = to_read_only(chan)
// // Rejected by the compiler.
// chan_ro |. send(5)