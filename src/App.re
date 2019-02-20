
let component = ReasonReact.statelessComponent(__MODULE__);

let make = _children => {
  ...component,
  render: _self => {
    <div>
      <Nav />
      <Router>
        ...{url =>
          switch (url.path) {
          | [] => <HomePage />
          | ["login"] => <LoginPage />
          | ["logout"] => <LogoutPage />
          | _ => <NotFoundPage />
          }
        }
      </Router>
    </div>;
  },
};