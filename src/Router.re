module Link = {
  let component = ReasonReact.statelessComponent("Link");

  let handleClick = (href, event) =>
    if (!ReactEvent.Mouse.defaultPrevented(event)) {
      ReactEvent.Mouse.preventDefault(event);
      ReasonReact.Router.push(href);
    };

  let make = (~href, ~className="", children) => {
    ...component,
    render: self =>
      <a
        href
        className
        onClick={self.handle((event, _) => handleClick(href, event))}>
        ...children
      </a>,
  };
};

type action =
  | ReceiveUrl(ReasonReact.Router.url);

type state = {url: ReasonReact.Router.url};

let component = ReasonReact.reducerComponent(__MODULE__);

let make = children => {
  ...component,
  initialState: () => {url: ReasonReact.Router.dangerouslyGetInitialUrl()},
  reducer: (action, _state) =>
    switch (action) {
    | ReceiveUrl(url) => ReasonReact.Update({url: url})
    },
  didMount: self => {
    let token =
      ReasonReact.Router.watchUrl(url => self.send(ReceiveUrl(url)));
    self.onUnmount(() => ReasonReact.Router.unwatchUrl(token));
  },
  render: self => children(self.state.url),
};