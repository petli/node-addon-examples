#define BUILDING_NODE_EXTENSION
#include <node.h>
#include "myobject.h"

using namespace v8;

MyObject::MyObject() {};
MyObject::~MyObject() {};

Persistent<Function> MyObject::constructor;
Persistent<Value> MyObject::prototype;

void MyObject::Init() {
  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("MyObject"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  // Prototype
  tpl->PrototypeTemplate()->Set(String::NewSymbol("plusOne"),
      FunctionTemplate::New(PlusOne)->GetFunction());

  constructor = Persistent<Function>::New(tpl->GetFunction());

  // Get hold of the prototype for our objects so we can do some
  // sanity checking on the objects the caller passes to us later
  Local<Object> obj = constructor->NewInstance();
  prototype = Persistent<Value>::New(obj->GetPrototype());
}

Handle<Value> MyObject::New(const Arguments& args) {
  HandleScope scope;

  MyObject* obj = new MyObject();
  obj->counter_ = args[0]->IsUndefined() ? 0 : args[0]->NumberValue();
  obj->Wrap(args.This());

  return args.This();
}

Handle<Value> MyObject::NewInstance(const Arguments& args) {
  HandleScope scope;

  const unsigned argc = 1;
  Handle<Value> argv[argc] = { args[0] };
  Local<Object> instance = constructor->NewInstance(argc, argv);

  return scope.Close(instance);
}

Handle<Value> MyObject::PlusOne(const Arguments& args) {
  HandleScope scope;

  MyObject* obj = CheckedUnWrap(args.This());
  if (obj) {
    obj->counter_ += 1;
    return scope.Close(Number::New(obj->counter_));
  }
  else {
    // Invalid type, an exception has been thrown so return an empty value
    return Handle<Value>();
  }
}


MyObject* MyObject::CheckedUnWrap(Handle<Object> handle)
{
  // Sanity check the object before we accept it as one of our own
  // wrapped objects
  
  // Basic checks done as asserts by UnWrap()
  if (!handle.IsEmpty() && handle->InternalFieldCount() == 1) {
    // Check the prototype.  This effectively stops inheritance,
    // but since this is created from a factory function and no
    // constructor is exposed that should not be ok.  If you really need
    // inheritance, turn this into a loop walking the prototype chain.
    Handle<Value> objproto = handle->GetPrototype();
    if (objproto == prototype) {
      // OK, this is us
      return ObjectWrap::Unwrap<MyObject>(handle);
    }
  }
  
  ThrowException(Exception::TypeError(String::New("<this> is not a MyObject")));
  return NULL;
}
