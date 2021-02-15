type time = Js.Date.t
type inet_addr = string

type connecting = {when_initiated: time};

type connected = {
  last_ping: option((time, int)),
  session_id: string
};

type disconnected = {when_disconnected: time};

type connection_state =
  | Connecting(connecting)
  | Connected(connected)
  | Disconnected(disconnected);

type connection_info = {
  state: connection_state,
  server: inet_addr
};
