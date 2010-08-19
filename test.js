var nas = require("./build/default/nas")
  , start = Date.now()

console.log("js "+(Date.now() - start), "before hello")
nas.doSomething(1, 2, "hello", function (er, res, n) {
  console.log("js "+(Date.now() - start), er, res, n)
})
// console.log(Date.now(), "before goodbye")
// nas.doSomething(3, 4, "goodbye", function (er, res, n) {
//   console.log(Date.now(), er, res, n)
// })
