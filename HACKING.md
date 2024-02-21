# Hacking
This is a document to explain how to use this project from the perspective of a
developer, and document design rationale alone the way


## Platform Independent Abstraction Layer
This project tries very hard to keep the platform specific layer separate from the actual
main logic code and think's of the platform layer as a service to provide the
main program with the resources it asks for, rather then calling a bunch of void
functions and having the platform layer do a bunch of magic. This is based on
the Handmade Hero style of hardware abstraction

```
Main Program -> Platform Specific Call
Main Logic   <- Platform Returns Generic Resources to Manage
```

The platform specific layer is setup with an interface-like macro system that
defines the required functions and properties that must exist within each
layer. This can optionally be accompanied be by an abstract class which is
disabled during testing code to avoid the complexities of vtables. There will
never a situation where you have to pick a different platform specific layer at
runtime in a very generic way so virtual classes are a completely unnecessary
complexity. Switch statements or typedefs are sufficient.
