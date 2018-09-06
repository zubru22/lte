/*BSD 3-Clause
Copyright (c) 2012, Armon Dadgar
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the organization nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL ARMON DADGAR BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/


#ifndef HASHMAP_H
#define HASHMAP_H

#ifndef MURMURHASH3_H
#include "MurmurHash3.h"
#endif

/**
 * Opaque hashmap reference
 */
typedef struct hashmap hashmap;
typedef int(*hashmap_callback)(void *data, const char *key, void *value);

/**
 * Creates a new hashmap and allocates space for it.
 * @arg initial_size The minimim initial size. 0 for default (64).
 * @arg map Output. Set to the address of the map
 * @return 0 on success.
 */
int hashmap_init(int initial_size, hashmap **map);

/**
 * Destroys a map and cleans up all associated memory
 * @arg map The hashmap to destroy. Frees memory.
 */
int hashmap_destroy(hashmap *map);

/**
 * Returns the size of the hashmap in items
 */
int hashmap_size(hashmap *map);

/**
 * Gets a value.
 * @arg key The key to look for. Must be null terminated.
 * @arg value Output. Set to the value of th key.
 * 0 on success. -1 if not found.
 */
int hashmap_get(hashmap *map, char *key, void **value);

/**
 * Puts a key/value pair.
 * @arg key The key to set. This is copied, and a seperate
 * version is owned by the hashmap. The caller the key at will.
 * @notes This method is not thread safe.
 * @arg key_len The key length
 * @arg value The value to set.
 * 0 if updated, 1 if added.
 */
int hashmap_put(hashmap *map, char *key, void *value);

/**
 * Deletes a key/value pair.
 * @notes This method is not thread safe.
 * @arg key The key to delete
 * @arg key_len The key length
 * 0 on success. -1 if not found.
 */
int hashmap_delete(hashmap *map, char *key);

/**
 * Clears all the key/value pairs.
 * @notes This method is not thread safe.
 * 0 on success. -1 if not found.
 */
int hashmap_clear(hashmap *map);

/**
 * Iterates through the key/value pairs in the map,
 * invoking a callback for each. The call back gets a
 * key, value for each and returns an integer stop value.
 * If the callback returns 1, then the iteration stops.
 * @arg map The hashmap to iterate over
 * @arg cb The callback function to invoke
 * @arg data Opaque handle passed to the callback
 * @return 0 on success, or the return of the callback.
 */
int hashmap_iter(hashmap *map, hashmap_callback cb, void *data);

#endif