/**
 * Copyright (c) 2018 Marek Benc <dusxmt@gmx.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* Support for checksum-based file comparison. */

#ifndef _RUDGIOSYNC_CHECKSUM_H_
#define _RUDGIOSYNC_CHECKSUM_H_

#include "boiler.h"

#ifdef RUDGIOSYNC_CHECKSUM_ENABLED
#include <nettle/sha2.h>

struct RudgiosyncChecksum_
{
  uint8_t sha256_digest[SHA256_DIGEST_SIZE];
};

typedef struct RudgiosyncChecksum_ RudgiosyncChecksum;
typedef struct sha256_ctx          RudgiosyncHashContext;

#else /* !RUDGIOSYNC_CHECKSUM_ENABLED */

/* Dummy type declarations. */
typedef gchar RudgiosyncChecksum;
typedef gchar RudgiosyncHashContext;

#endif /* !RUDGIOSYNC_CHECKSUM_ENABLED */


/* Initialize the hashing state, to be allocated statically. */
void rudgiosync_hash_init (RudgiosyncHashContext *context);

/* Hash a chunk of data */
void rudgiosync_hash_data (RudgiosyncHashContext *context,
                           gsize length, const gchar *data);

/* Produce a checksum from the hashed data, to be allocated statically. */
void rudgiosync_hash_finish (RudgiosyncHashContext *context,
                             RudgiosyncChecksum *checksum);

/* Displays the given checksum in a human readable form. */
void rudgiosync_checksum_display (RudgiosyncChecksum *checksum);


/* Produce a checksum for a GFile, to be allocated statically. */
gboolean rudgiosync_checksum_for_gfile (GFile *descriptor,
                                        RudgiosyncChecksum *checksum,
                                        GError **error);

/* Compare two checksums, returning true if they differ. */
gboolean rudgiosync_checksums_differ (RudgiosyncChecksum *checksum_a,
                                      RudgiosyncChecksum *checksum_b);


#endif /* _RUDGIOSYNC_CHECKSUM_H_ */
