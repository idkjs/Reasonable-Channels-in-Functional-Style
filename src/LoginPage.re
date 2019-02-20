let component = ReasonReact.statelessComponent("Login");

let make = (_children) => {
  ...component,
  render: _self => <div> (ReasonReact.string("Login Page! ")) </div>,
};