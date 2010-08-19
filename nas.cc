
#include <v8.h>
#include <node.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;
using namespace node;
using namespace v8;

static Handle<Value> DoSomethingAsync (const Arguments&);
static int DoSomething (eio_req *);
static int DoSomething_After (eio_req *);
extern "C" void init (Handle<Object>);

struct simple_request {
  int x;
  int y;
  Persistent<Function> cb;
};

static Handle<Value> DoSomethingAsync (const Arguments& args) {
  HandleScope scope;
  const char *usage = "usage: doSomething(x, y, cb)";
  if (args.Length() != 3) {
    return ThrowException(Exception::Error(String::New(usage)));
  }
  int x = args[0]->Int32Value();
  int y = args[1]->Int32Value();
  fprintf(stderr, "      >>>DoSomethingAsync %d %d\n", x, y);
  Local<Function> cb = Local<Function>::Cast(args[2]);
  
  simple_request *sr = (simple_request *)
    malloc(sizeof(struct simple_request) + 1);
  
  sr->cb = Persistent<Function>::New(cb);
  sr->x = x;
  sr->y = y;

  fprintf(stderr, "      >>>about to eio_custom, sr=%d\n", sr);
  eio_custom(DoSomething, EIO_PRI_DEFAULT, DoSomething_After, sr);
  ev_ref(EV_DEFAULT_UC);
  fprintf(stderr, "      >>>returning\n");
  return Undefined();
}

// this function happens on the thread pool
static int DoSomething (eio_req *req) {
  fprintf(stderr, "      >>>DoSomething %d,%d\n", req, req->data);
  struct simple_request * sr = (struct simple_request *)req->data;
  // note that this is always 1024 on Solaris, not some big number.
  // It's like it loses the req->data somewhere along the way.  ?
  fprintf(stderr, "      >>>sr pointer %d\n", sr);
  fprintf(stderr, "      >>>about to sleep\n");
  sleep(2);
  fprintf(stderr, "      >>>read req, about to set result\n");
  // just READING the data seems to blow it up.
  fprintf(stderr, "      >>>x %d\n", sr->x);
  fprintf(stderr, "      >>>y %d\n", sr->y);
  // Why does this crash in Solaris?
  req->result = sr->x + sr->y;
  fprintf(stderr, "      >>>returning\n");
  return 0;
}

static int DoSomething_After (eio_req *req) {
  fprintf(stderr, "      >>>DoSomething_After %d\n", req);
  HandleScope scope;
  ev_unref(EV_DEFAULT_UC);
  struct simple_request * sr = (struct simple_request *)req->data;
  Local<Value> argv[2];
  argv[0] = Local<Value>::New(Null());
  argv[1] = Integer::New(req->result);
  TryCatch try_catch;
  sr->cb->Call(Context::GetCurrent()->Global(), 2, argv);
  if (try_catch.HasCaught()) {
    FatalException(try_catch);
  }
  sr->cb.Dispose();
  free(sr);
  return 0;
}

extern "C" void init (Handle<Object> target) {
  HandleScope scope;
  NODE_SET_METHOD(target, "doSomething", DoSomethingAsync);
}
