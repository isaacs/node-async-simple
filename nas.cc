
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
  char name[1];
  Persistent<Function> cb;
};

static Handle<Value> DoSomethingAsync (const Arguments& args) {
  HandleScope scope;
  const char *usage = "usage: doSomething(x, y, name, cb)";
  if (args.Length() != 4) {
    return ThrowException(Exception::Error(String::New(usage)));
  }
  int x = args[0]->Int32Value();
  int y = args[1]->Int32Value();
  String::Utf8Value name(args[2]);
  fprintf(stderr, "      >>>DoSomethingAsync %d %d %s\n", x, y, *name);
  Local<Function> cb = Local<Function>::Cast(args[3]);
  
  simple_request *sr = (simple_request *)
    malloc(sizeof(struct simple_request) + name.length() + 1);
  
  sr->cb = Persistent<Function>::New(cb);
  strncpy(sr->name, *name, name.length() + 1);
  sr->x = x;
  sr->y = y;

  fprintf(stderr, "      >>>about to eio_custom\n");
  eio_custom(DoSomething, EIO_PRI_DEFAULT, DoSomething_After, sr);
  ev_ref(EV_DEFAULT_UC);
  fprintf(stderr, "      >>>returning\n");
  return Undefined();
}

// this function happens on the thread pool
static int DoSomething (eio_req *req) {
  fprintf(stderr, "      >>>DoSomething %d\n", req);
  struct simple_request * sr = (struct simple_request *)req->data;
  fprintf(stderr, "      >>>about to sleep\n");
  sleep(2);
  fprintf(stderr, "      >>>read req, about to set result\n");
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
  Local<Value> argv[3];
  argv[0] = Local<Value>::New(Null());
  argv[1] = Integer::New(req->result);
  argv[2] = String::New(sr->name);
  TryCatch try_catch;
  sr->cb->Call(Context::GetCurrent()->Global(), 3, argv);
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
