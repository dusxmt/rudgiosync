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

/* File and directory manipulation routines. */

#ifndef _RUDGIOSYNC_OPERATIONS_H_
#define _RUDGIOSYNC_OPERATIONS_H_

#include "boiler.h"
#include "descriptions.h"

void traverse_directory_tree (RudgiosyncDirectoryEntry *entry,
                              const gchar *prefix);

/* Delete the given directory entry.  May fail, but entry is always freed. */
gboolean rudgiosync_directory_entry_delete (RudgiosyncDirectoryEntry *entry, 
                                            GError **error);

gboolean rudgiosync_synchronize (RudgiosyncDirectoryEntry **destination,
                                 RudgiosyncDirectoryEntry **source,
                                 gboolean size_only,
                                 gboolean checksum_only,
                                 gboolean delete_unwanted,
                                 GError **error);

#endif /* _RUDGIOSYNC_OPERATIONS_H_ */
