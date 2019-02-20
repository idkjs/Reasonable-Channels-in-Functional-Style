let component = ReasonReact.statelessComponent("NotFoundPage");

let make = (_children) => {
  ...component,
  render: _self => <div> (ReasonReact.string("NotFoundPage! ")) </div>,
};