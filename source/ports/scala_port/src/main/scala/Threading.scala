// import java.util.concurrent.BlockingQueue
// import java.util.concurrent.atomic.AtomicBoolean
// import metacall.{Caller, Value}

// Due to Runtime constraints, usually most languages need to be executed in the Main thread.
// Also, it is not possible to call simultaneously between multiple threads, so meanwhile
// core does not support full threads with atomics, we can use this technique:
// We will run one consumer (in the main thread, which will be blocking) and multiple
// producers (in different threads) which are supposed to be the worker threads of the
// event loop of some random framework for handling networking.


// TODO: Implement here the logic of the MetaCallSpec parallel test

