open Channel;

let end' = _ => ()  /* = ignore from Pervasives */

let filter = (source, pred) => {
    let target = create()
    source |. listen( m => 
        pred(m) ? target |. send(m) |. end' : ()
    ) 
    to_read_only(target)
}

let chan = create() 

chan |. filter(i => i > 0) 
     |. listen(i => Js.log(i)) 
     |. end'

chan |. send(1) 
     |. send(-1) 
     |. end' 

chan |. send(2) |. end'
