// Generated by BUCKLESCRIPT, PLEASE EDIT WITH CARE
'use strict';

var Curry = require("bs-platform/lib/js/curry.js");
var Channel$ChannelMedium = require("./Channel.bs.js");

function end$prime(param) {
  
}

function filter(source, pred) {
  var target = Channel$ChannelMedium.create(undefined);
  Channel$ChannelMedium.listen(source, (function (m) {
          if (Curry._1(pred, m)) {
            Channel$ChannelMedium.send(target, m);
            return ;
          }
          
        }));
  return Channel$ChannelMedium.to_read_only(target);
}

var chan = Channel$ChannelMedium.create(undefined);

Channel$ChannelMedium.listen(filter(chan, (function (i) {
            return i > 0;
          })), (function (i) {
        console.log("i>0", i);
        
      }));

Channel$ChannelMedium.send(Channel$ChannelMedium.send(Channel$ChannelMedium.send(Channel$ChannelMedium.send(chan, 1), -1), 13), 4);

Channel$ChannelMedium.send(chan, 2);

exports.end$prime = end$prime;
exports.filter = filter;
exports.chan = chan;
/* chan Not a pure module */