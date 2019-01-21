# libsfuid
Semantic-Free Unique ID En-/Decoder Lib

## What is this?

This is a lib that encodes enumerated IDs into a seemingly random string of characters.

Possible usecases include the obscuration of internal IDs in a system for user generated content so that no information about 
the existence of other postings is leaked to attackers.

## So this is a library for hashing?

Not quite. Although there are certain similarities.
The biggest difference is that a hash is in general not reverseable. The encoded IDs of this library are. This might be useful 
because no redundant data has to be stored.
By extension this means that there are no collisions in the generated IDs. Thus the input range of the encoder is limited.

## How is this useful if the input range is limited?

Well, by default the lib provides about 576 quadrillion unique IDs (2^59). If more are needed the settings can be tuned to get 
about 18 quintillion (2^64) unique IDs - at the expense of not using the output space efficiently.

If you really need more then message me and I'll try to upgrade to 128 bit.

## What about timing?

I think I got it working pretty fast. On my laptop (i7-4600U) encoding takes about 500 ns. Decoding is a bit slower and takes 
about 2 µs. I provided the benchmark script I used in case you want to check on your own machine.

## Does this work everywhere?

Probably not. The lib needs a 128 bit integer type. So I don't think this is working on any 32 bit machines. Also the type 
definition I used is specific for GCC. If you get it to work on another compiler, please tell me.

## How can I use it.

Oh, that's pretty simple.

```
sfuid_settings_t settings = sfuid_default_settings;
sfuid_init(settings);

// Now we can use it.

char* string = malloc(settings.length + 1);
sfuid_encode(42, string);

uint64_t result;
sfuid_decode(string, &result);
```
