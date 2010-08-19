var nas = require("./build/default/nas")
  , sys = require("sys")

if (typeof console === "undefined") {
  global.console = { log : function () {
    sys.debug(sys.inspect([].slice.call(arguments)))
  }}
}


console.log(Date.now(), "before hello")
nas.doSomething(1, 2, "hello", function (er, res, n) {
  console.log(Date.now(), er, res, n)
})
console.log(Date.now(), "before goodbye")
nas.doSomething(3, 4, "goodbye", function (er, res, n) {
  console.log(Date.now(), er, res, n)
})
