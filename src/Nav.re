  module Link = Router.Link;
  let component = ReasonReact.statelessComponent("Nav");
  let str = ReasonReact.string;
  let make = _children => {
    ...component,
    render: _self =>
      <div>
        <ul className="nav">
          <li>
            <Link href="/"> {ReasonReact.string("clicking here!")} </Link>
          </li>
          <li>
            <Link href="login"> {ReasonReact.string("Go To Login")} </Link>
          </li>
          <li>
            <Link href="/logout"> {ReasonReact.string("Logout")} </Link>
          </li>
        </ul>
      </div>,
  };