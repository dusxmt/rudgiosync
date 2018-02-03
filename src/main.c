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

static gboolean opt_delete    = FALSE;
static gboolean opt_checksum  = FALSE;
static gboolean opt_size_only = FALSE;

static GOptionEntry opt_entries[] =
{
  { "size-only", 's', 0, G_OPTION_ARG_NONE, &opt_size_only, "Skip files that match in size", NULL },
  { "checksum",  'c', 0, G_OPTION_ARG_NONE, &opt_checksum,  "Skip files based on checksum, not size and modified time", NULL },
  { "delete",    'd', 0, G_OPTION_ARG_NONE, &opt_delete,    "Delete extraneous files from destination directories", NULL },
  { NULL }
};

int
main (int argc, char **argv)
{
  GOptionContext *opt_context;
  RudgiosyncDirectoryEntry *source;
  RudgiosyncDirectoryEntry *destination;

  const gchar *src_uri;
  const gchar *dest_uri;

  GError *ierror = NULL;


  g_type_init();
  opt_context = g_option_context_new ("<source> <destination>");
  g_option_context_set_summary (opt_context,
                               "rudgiosync is a simplistic file synchronizing utility, inspired heavily by\n"
                               "rsync, which uses GIO as its I/O library, and can therefore natively access\n"
                               "and synchronize gvfs-based filesystems.\n"
                               "\n"
                               "The <source> and <destination> arguments must be valid GIO-supported URIs.");
  g_option_context_add_main_entries (opt_context, opt_entries, NULL);
  g_option_context_parse (opt_context, &argc, &argv, &ierror);
  if (ierror != NULL)
    {
      g_printerr ("%s: Command line option parsing failed: %s.\n", g_get_prgname (), ierror->message);
      g_clear_error (&ierror);
      return 1;
    }

  if (argc < 2)
    {
      g_printerr ("%s: Command line option parsing failed: %s.\n", g_get_prgname (), "Source location missing");
      return 1;
    }
  src_uri = argv[1];

  if (argc < 3)
    {
      g_printerr ("%s: Command line option parsing failed: %s.\n", g_get_prgname (), "Destination location missing");
      return 1;
    }
  dest_uri = argv[2];

  if (argc > 3)
    {
      g_printerr ("%s: Command line option parsing failed: Unknown option `%s'.\n", g_get_prgname (), argv[3]);
      return 1;
    }


  g_print ("Examining source directory tree... ");
  source = rudgiosync_directory_entry_new (src_uri, FALSE, &ierror);
  g_print ("done.\n");
  if (ierror != NULL)
    {
      g_printerr ("%s: Failed to investigate the source: %s.\n", g_get_prgname (), ierror->message);
      return 1;
    }

  g_print ("Examining destination directory tree... ");
  destination = rudgiosync_directory_entry_new (dest_uri, FALSE, &ierror);
  g_print ("done.\n");
  if (ierror != NULL)
    {
      g_printerr ("%s: Failed to investigate the destination: %s.\n", g_get_prgname (), ierror->message);
      return 1;
    }

  rudgiosync_synchronize (&destination, &source, !opt_size_only, opt_checksum, opt_delete, &ierror);
  if (ierror != NULL)
    {
      g_printerr ("%s: Synchronization failed: %s.\n", g_get_prgname (), ierror->message);
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
