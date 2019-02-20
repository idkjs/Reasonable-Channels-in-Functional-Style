let component = ReasonReact.statelessComponent("Logout");

let make = (_children) => {
  ...component,
  render: _self => <div> (ReasonReact.string("Logout Page! ")) </div>,
};