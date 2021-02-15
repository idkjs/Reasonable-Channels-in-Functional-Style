// https://queue.acm.org/detail.cfm?id=2038036
let rec destutter = (l) =>
  switch l {
  | [] => []
  | [x] => [x]
  | [x, y, ...rest] =>
    if (x == y) {
      destutter([y, ...rest]);
    } else {
      [x, ...destutter([y, ...rest])];
    }
  };

type connection_state =
  | Connecting
  | Connected
  | Disconnected;
type time = Js.Date.t
type inet_addr = string
type connection_info = {
  state: connection_state,
  server: inet_addr,
  last_ping_time: option(time),
  last_ping_id: option(int),
  session_id: option(string),
  when_initiated: option(time),
  when_disconnected: option(time)
};
