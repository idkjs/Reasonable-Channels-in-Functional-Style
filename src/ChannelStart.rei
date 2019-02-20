// Internal representation of Channel type, polymorphic on type variable 'a.
type t('a)

// Create a new channel that carries values of type 'a.
let create: unit => t('a)

// Send a value of type 'a on a channel of type t('a)
let send: t('a) => 'a => t('a)


// Receive a value of type 'a, handled by a function of type 'a => unit.
let recv: t('a) => ('a => unit) => t('a)

// As above, but continue to listen for messages.
let listen: t('a) => ('a => unit) => unit

// Receive if there is a value, synchronously.
let recv_sync: t('a) => option('a)
