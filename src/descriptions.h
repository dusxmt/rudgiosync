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

/* Structures used to describe files and directories. */

#ifndef _RUDGIOSYNC_DESCRIPTIONS_H_
#define _RUDGIOSYNC_DESCRIPTIONS_H_

#include "boiler.h"


typedef struct RudgiosyncFile_ RudgiosyncFile;
typedef struct RudgiosyncDirectory_ RudgiosyncDirectory;
typedef struct RudgiosyncDirectoryEntry_ RudgiosyncDirectoryEntry;

struct RudgiosyncFile_
{
  guint64  size; 
  gchar   *checksum;
};

struct RudgiosyncDirectory_
{
  GSList *entries;   /* of type RudgiosyncDirectoryEntry */
};

enum
{
  RUDGIOSYNC_DIR_ENTRY_FILE,
  RUDGIOSYNC_DIR_ENTRY_DIR,
  RUDGIOSYNC_DIR_ENTRY_OTHER
};

struct RudgiosyncDirectoryEntry_
{
  guint   type;
  GFile  *descriptor;
  gchar  *name;
  gchar  *display_name;
  guint64 modified_time;

  union
    {
      RudgiosyncFile file;
      RudgiosyncDirectory directory;
    } data;
};


RudgiosyncDirectoryEntry *rudgiosync_directory_entry_new (const gchar *uri, gboolean checksum_wanted, GError **error);

void rudgiosync_directory_entry_free (gpointer to_free);


#endif /* _RUDGIOSYNC_DESCRIPTIONS_H_ */
