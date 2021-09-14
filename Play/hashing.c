// - https://en.m.wikipedia.org/wiki/MurmurHash

long murmur64(long h)
  h ^= h >>> 33;
  h *= 0xff51afd7ed558ccdL;
  h ^= h >>> 33;
  h *= 0xc4ceb9fe1a85ec53L;
  h ^= h >>> 33;
  return h;
}

// https://www.crockford.com/fash/fash64.html
// https://www.partow.net/programming/hashfunctions/
// https://www.partow.net/programming/hashfunctions/#AvailableHashFunctions

// overall good (for MSB variations)
unsigned int SDBMHash(const char* str, unsigned int length)
{
   unsigned int hash = 0;
   unsigned int i    = 0;

   for (i = 0; i < length; ++str, ++i)
   {
      hash = (*str) + (hash << 6) + (hash << 16) - hash;
   }

   return hash;
}

// most efficent
unsigned int DJBHash(const char* str, unsigned int length)
{
   unsigned int hash = 5381;
   unsigned int i    = 0;

   for (i = 0; i < length; ++str, ++i)
   {
      hash = ((hash << 5) + hash) + (*str);
   }

   return hash;
}

// DEK - Donald E Knuth
unsigned int DEKHash(const char* str, unsigned int length)
{
   unsigned int hash = len;
   unsigned int i    = 0;

   for (i = 0; i < length; ++str, ++i)
   {
      hash = ((hash << 5) ^ (hash >> 27)) ^ (*str);
   }

   return hash;
}

// https://en.m.wikipedia.org/wiki/List_of_hash_functions


// - https://en.m.wikipedia.org/wiki/SipHash
// https://github.com/veorq/SipHash
 
// Cryptographically secure
// (used in perl and many OSs
// - https://github.com/veorq/SipHash/blob/master/halfsiphash.c
