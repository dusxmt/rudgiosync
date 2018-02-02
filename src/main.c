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

#include "boiler.h"
#include "descriptions.h"
#include "operations.h"

int
main (int argc, char **argv)
{
  RudgiosyncDirectoryEntry *source;
  RudgiosyncDirectoryEntry *destination;
  GError *ierror = NULL;

  if (argc != 3)
    {
      g_printerr ("%s: Exactly two arguments are requested.\n", argv[0]);
      return 1;
    }
  g_type_init();

  g_print ("Examining source directory tree... ");
  source = rudgiosync_directory_entry_new (argv[1], FALSE, &ierror);
  g_print ("done.\n");
  if (ierror != NULL)
    {
      g_printerr ("%s: Failed to investigate the source: %s.\n", argv[0], ierror->message);
      return 1;
    }

  g_print ("Examining destination directory tree... ");
  destination = rudgiosync_directory_entry_new (argv[2], FALSE, &ierror);
  g_print ("done.\n");
  if (ierror != NULL)
    {
      g_printerr ("%s: Failed to investigate the destination: %s.\n", argv[0], ierror->message);
      return 1;
    }

  rudgiosync_synchronize (&destination, &source, TRUE, FALSE, FALSE, &ierror);
  if (ierror != NULL)
    {
      g_print ("Synchronization failed: %s.\n", ierror->message);
      g_clear_error (&ierror);
    }
  /*
  traverse_directory_tree (source, NULL);
  traverse_directory_tree (destination, NULL);
  */

  rudgiosync_directory_entry_free (source);
  rudgiosync_directory_entry_free (destination);

  return 0;
}
