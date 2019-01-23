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

## How can I use it?

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

## How does this work?

It's basically a kind of MCG (multiplicative congruential generator) but instead of the last entry in the series being the parameter I used the current ID. That means the result is not as random because the distance between consecutive ID is basically constant. The reason I did this is to save computation time (exponentiation would work, but 128 bit won't be enough for numbers that size).

So the formular to get a specific ID is: 
s(i) := p\*i mod m

The condition for p and m is that their GCD is 1. I chose m to be a power of 2 so it's prime factors are only 2s. For p I chose a prime that is about 25 % of m. For that I hardcoded a list of possible primes candidates into the program.

To make it more efficient the program calculates m to be the greatest power of 2 that fits in the output space.
For example: Let the character set be "0-9" (10 characters) and the length of the result is 4, the output space would be 10^4. The biggest power of 2 in that space is 2^13. That's m. The p would be 2039 (about 25 % of m).

(Note to myself: Maybe just using a big Mersenne prime (like 2^61-1) would also work. That would maximize the useable output space efficiency, because it's guaranteed that x^y with y > 1 is not a prime, thus m can be size of the output space.)

The result of the MCG is then converted into the string by treating it as a number with the length of the charset as its basis.

As for the decoding we have to solve the congruence equation x == p * i (mod m) for i. We can do that by multiplying x the multiplicative inverse of p and m.
