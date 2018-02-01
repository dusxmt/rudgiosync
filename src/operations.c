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
#include "operations.h"
#include "errors.h"

#include <string.h>

#define TRANSFER_BUF_SIZE ((gsize)(2 * 1024 * 1024)) /* 2 MiB */


static void
traverse_directory_tree_p (gpointer entry_in, gpointer prefix_in)
{
  RudgiosyncDirectoryEntry *entry = (RudgiosyncDirectoryEntry *)entry_in;
  const gchar *prefix = (const gchar *)prefix_in;

  traverse_directory_tree (entry, prefix);
}

void
traverse_directory_tree (RudgiosyncDirectoryEntry *entry, const gchar *prefix)
{
  gchar *printed_name;

  if (prefix != NULL)
    {
      printed_name = g_strdup_printf ("%s/%s", prefix, entry->display_name);
    }
  else
    {
      printed_name = g_strdup (entry->display_name);
    }
  g_print ("%s/", printed_name);

  switch (entry->type)
    {
      case RUDGIOSYNC_DIR_ENTRY_FILE:
        if (entry->data.file.checksum != NULL)
          {
            g_print (" (file, size: %" G_GUINT64_FORMAT ", checksum: %s)\n", entry->data.file.size, entry->data.file.checksum);
          }
        else
          {
            g_print (" (file, size: %" G_GUINT64_FORMAT ")\n", entry->data.file.size);
          }
        break;

      case RUDGIOSYNC_DIR_ENTRY_DIR:
        g_print ("/ (directory)\n");

        g_slist_foreach (entry->data.directory.entries, traverse_directory_tree_p, printed_name);
        break;

      default:
        g_print (" (other)\n");
    }

  g_free (printed_name);
}

gboolean
rudgiosync_directory_entry_delete (RudgiosyncDirectoryEntry *entry,
                                   GError **error)
{
  RudgiosyncDirectoryEntry *child_entry;
  GSList *list_iter;
  gchar  *entry_uri;
  GError *ierror = NULL;
  

  if (entry->type == RUDGIOSYNC_DIR_ENTRY_DIR)
    {
      for (list_iter = entry->data.directory.entries;
           list_iter != NULL;
           list_iter = list_iter->next)
        {
          child_entry = (RudgiosyncDirectoryEntry *)(list_iter->data);
          rudgiosync_directory_entry_delete (child_entry, &ierror);
          list_iter->data = NULL;

          if (ierror != NULL)
            {
              g_propagate_error (error, ierror);
              rudgiosync_directory_entry_free (entry);
              return FALSE;
            }
        }
    }

  entry_uri = g_file_get_uri (entry->descriptor);
  g_file_delete (entry->descriptor, NULL, &ierror);
  rudgiosync_directory_entry_free (entry);

  if (ierror != NULL)
    {
      g_propagate_prefixed_error (error, ierror, "Failed to delete `%s': ", entry_uri);
      g_free (entry_uri);
      return FALSE;
    }

  g_print ("Deleted `%s'.\n", entry_uri);
  g_free (entry_uri);
  return TRUE;
}

static gboolean
delete_non_present_entries_from_dest (RudgiosyncDirectoryEntry *destination,
                                      RudgiosyncDirectoryEntry *source,
                                      GError **error)
{
  RudgiosyncDirectoryEntry *dest_entry;
  RudgiosyncDirectoryEntry *src_entry;

  GSList *dest_entry_li;
  GSList *src_entry_li;

  gboolean found;

  GError *ierror = NULL;


  g_assert (destination->type == RUDGIOSYNC_DIR_ENTRY_DIR);
  g_assert (source->type == RUDGIOSYNC_DIR_ENTRY_DIR);

  for (dest_entry_li = destination->data.directory.entries;
       dest_entry_li != NULL;
       dest_entry_li = dest_entry_li->next)
    {
      dest_entry = (RudgiosyncDirectoryEntry *)(dest_entry_li->data);
      found = FALSE;

      for (src_entry_li = source->data.directory.entries;
           src_entry_li != NULL && !found;
           src_entry_li = src_entry_li->next)
        {
          src_entry = (RudgiosyncDirectoryEntry *)(src_entry_li->data);

          if (strcmp (src_entry->name, dest_entry->name) == 0)
            found = TRUE;
        }
      if (!found)
        {
          rudgiosync_directory_entry_delete (dest_entry, &ierror);
          dest_entry_li->data = NULL;
          if (ierror != NULL)
            {
              g_propagate_error (error, ierror);
              destination->data.directory.entries = g_slist_remove_all (destination->data.directory.entries, NULL);
              return FALSE;
            }
        }
    }

  destination->data.directory.entries = g_slist_remove_all (destination->data.directory.entries, NULL);
  return TRUE;
}

static RudgiosyncDirectoryEntry *
create_empty_file (GFile *descriptor, gboolean checksum_wanted, GError **error)
{
  RudgiosyncDirectoryEntry *retval;
  GFileOutputStream        *output_stream;
  gchar                    *uri;

  GError *ierror = NULL;


  output_stream = g_file_create (descriptor,
                                 G_FILE_CREATE_REPLACE_DESTINATION,
                                 NULL,
                                 &ierror);
  if (ierror != NULL)
    {
      g_propagate_error (error, ierror);
      return NULL;
    }
  g_object_unref (output_stream);

  uri = g_file_get_uri (descriptor);
  retval = rudgiosync_directory_entry_new (uri, checksum_wanted, &ierror);
  g_free (uri);

  if (ierror != NULL)
    {
      g_propagate_error (error, ierror);
      return NULL;
    }

  return retval;
}

static gboolean
files_differ (RudgiosyncDirectoryEntry *destination,
              RudgiosyncDirectoryEntry *source,
              gboolean size_only, gboolean checksum_only)
{
  if (size_only)
    {
      return destination->data.file.size != source->data.file.size;
    }
  else if (checksum_only)
    {
      g_warning ("Checksum comparison not yet supported, falling back to size-only.");
      return destination->data.file.size != source->data.file.size;
    }
  else
    {
      /* Here, we would check the modified timestamp and size. */
      return destination->data.file.size != source->data.file.size;
    }
}

/* Forward declaration. */
static gboolean rudgiosync_synchronize_internal (RudgiosyncDirectoryEntry **destination,
                                                 RudgiosyncDirectoryEntry **source,
                                                 gboolean size_only,
                                                 gboolean checksum_only,
                                                 gboolean delete_unwanted,
                                                 const gchar *prefix,
                                                 GError **error);


static gboolean
sync_file (RudgiosyncDirectoryEntry *destination,
           RudgiosyncDirectoryEntry *source,
           gboolean size_only,
           gboolean checksum_only,
           const gchar *prefix,
           gboolean already_modified,
           GError **error)
{
  GFileInputStream *input_stream;
  GFileOutputStream *output_stream;

  gchar *src_uri;
  gchar *dest_uri;

  gchar *transfer_buf;
  gboolean modified;

  gssize read_count;
  gsize wrote_count;

  GError *ierror = NULL;


  g_assert (source->type == RUDGIOSYNC_DIR_ENTRY_FILE);
  g_assert (destination->type == RUDGIOSYNC_DIR_ENTRY_FILE);

  modified = files_differ (destination, source, size_only, checksum_only);

  if (already_modified || modified)
    {
      if (prefix != NULL)
        g_print ("%s/%s\n", prefix, destination->display_name);
      else
        g_print ("%s\n", destination->display_name);
    }

  if (modified)
    {
      input_stream = g_file_read (source->descriptor, NULL, &ierror);
      if (ierror != NULL)
        {
          src_uri = g_file_get_uri (source->descriptor);
          dest_uri = g_file_get_uri (destination->descriptor);
          g_propagate_prefixed_error (error, ierror, "Failed to update `%s' with `%s': ", dest_uri, src_uri);
          g_free (src_uri);
          g_free (dest_uri);

          return FALSE;
        }

      output_stream = g_file_replace (destination->descriptor,
                                      NULL,
                                      FALSE,
                                      G_FILE_CREATE_NONE,
                                      NULL,
                                      &ierror);
      if (ierror != NULL)
        {
          src_uri = g_file_get_uri (source->descriptor);
          dest_uri = g_file_get_uri (destination->descriptor);
          g_propagate_prefixed_error (error, ierror, "Failed to update `%s' with `%s': ", dest_uri, src_uri);
          g_free (src_uri);
          g_free (dest_uri);

          return FALSE;
        }

      transfer_buf = g_new (gchar, TRANSFER_BUF_SIZE);
      while (TRUE)
        {
          read_count = g_input_stream_read (G_INPUT_STREAM (input_stream),
                                            transfer_buf, TRANSFER_BUF_SIZE,
                                            NULL, &ierror);
          if (ierror != NULL)
            {
              g_free (transfer_buf);
              g_object_unref (input_stream);
              g_object_unref (output_stream);

              src_uri = g_file_get_uri (source->descriptor);
              dest_uri = g_file_get_uri (destination->descriptor);
              g_propagate_prefixed_error (error, ierror, "Failed to update `%s' with `%s': ", dest_uri, src_uri);
              g_free (src_uri);
              g_free (dest_uri);

              return FALSE;
            }

          if (read_count == 0)
            break;

          g_output_stream_write_all (G_OUTPUT_STREAM (output_stream),
                                     transfer_buf, (gsize)read_count,
                                     &wrote_count,
                                     NULL,
                                     &ierror);
          if (ierror != NULL)
            {
              g_free (transfer_buf);
              g_object_unref (input_stream);
              g_object_unref (output_stream);

              src_uri = g_file_get_uri (source->descriptor);
              dest_uri = g_file_get_uri (destination->descriptor);
              g_propagate_prefixed_error (error, ierror, "Failed to update `%s' with `%s': ", dest_uri, src_uri);
              g_free (src_uri);
              g_free (dest_uri);

              return FALSE;
            }
        }
      g_free (transfer_buf);
      g_object_unref (input_stream);
      g_object_unref (output_stream);
    }

  return TRUE;
}

static gboolean
sync_directory (RudgiosyncDirectoryEntry *destination,
                RudgiosyncDirectoryEntry *source,
                gboolean size_only,
                gboolean checksum_only,
                gboolean delete_unwanted,
                const gchar *prefix,
                gboolean already_modified,
                GError **error)
{
  RudgiosyncDirectoryEntry *src_entry;
  RudgiosyncDirectoryEntry *dest_entry;

  GSList *src_entry_li;
  GSList *dest_entry_li;

  gchar *src_uri;
  gchar *dest_uri;

  gboolean found;

  GFile *temp_descriptor;
  gchar *dest_entry_prefix;

  GError *ierror = NULL;


  g_assert (source->type == RUDGIOSYNC_DIR_ENTRY_DIR);
  g_assert (destination->type == RUDGIOSYNC_DIR_ENTRY_DIR);

  if (prefix != NULL)
    dest_entry_prefix = g_strdup_printf ("%s/%s", prefix, destination->display_name);
  else
    dest_entry_prefix = g_strdup (destination->display_name);

  /**
   * To be symmetrical with sync_file, this routine has an already_modified
   * argument.  Unlike sync_file, we handle the argument right at the beginning,
   * since we don't consider adding/deleting entries to be modifying the
   * directory itself.
   */
  if (already_modified)
    g_print ("%s/\n", dest_entry_prefix);

  for (src_entry_li = source->data.directory.entries;
       src_entry_li != NULL;
       src_entry_li = src_entry_li->next)
    {
      src_entry = (RudgiosyncDirectoryEntry *)(src_entry_li->data);
      found = FALSE;

      for (dest_entry_li = destination->data.directory.entries;
           dest_entry_li != NULL && !found;
           dest_entry_li = dest_entry_li->next)
        {
          dest_entry = (RudgiosyncDirectoryEntry *)(dest_entry_li->data);

          if (strcmp (src_entry->name, dest_entry->name) == 0)
            {
              rudgiosync_synchronize_internal (&dest_entry, &src_entry,
                                               size_only, checksum_only, delete_unwanted,
                                               dest_entry_prefix,
                                               &ierror);

              dest_entry_li->data = (gpointer)dest_entry;
              src_entry_li->data  = (gpointer)src_entry;

              if (ierror != NULL)
                {
                  g_propagate_error (error, ierror);
                  g_free (dest_entry_prefix);
                  return FALSE;
                }

              found = TRUE;
            }
        }
      if (!found)
        {
          temp_descriptor = g_file_get_child (destination->descriptor, src_entry->name);
          switch (src_entry->type)
            {
              case RUDGIOSYNC_DIR_ENTRY_FILE:
                dest_entry = create_empty_file (temp_descriptor, checksum_only, &ierror);
                g_object_unref (temp_descriptor);
                if (ierror != NULL)
                  {
                    g_propagate_error (error, ierror);
                    g_free (dest_entry_prefix);
                    return FALSE;
                  }
                sync_file (dest_entry, src_entry,
                           size_only, checksum_only,
                           dest_entry_prefix,
                           TRUE,
                           &ierror);

                if (ierror != NULL)
                  {
                    g_propagate_error (error, ierror);
                    rudgiosync_directory_entry_free (dest_entry);
                    g_free (dest_entry_prefix);
                    return FALSE;
                  }

                destination->data.directory.entries = g_slist_prepend (destination->data.directory.entries, dest_entry);
                break;

              case RUDGIOSYNC_DIR_ENTRY_DIR:
                g_file_make_directory (temp_descriptor, NULL, &ierror);
                if (ierror != NULL)
                  {
                    g_propagate_error (error, ierror);
                    g_object_unref (temp_descriptor);
                    g_free (dest_entry_prefix);
                    return FALSE;
                  }
                dest_uri = g_file_get_uri (temp_descriptor);
                g_object_unref (temp_descriptor);
                dest_entry = rudgiosync_directory_entry_new (dest_uri, checksum_only, &ierror);
                g_free (dest_uri);
                if (ierror != NULL)
                  {
                    g_propagate_error (error, ierror);
                    g_free (dest_entry_prefix);
                    return FALSE;
                  }
                sync_directory (dest_entry, src_entry,
                                size_only, checksum_only, delete_unwanted,
                                dest_entry_prefix,
                                TRUE,
                                &ierror);

                if (ierror != NULL)
                  {
                    g_propagate_error (error, ierror);
                    rudgiosync_directory_entry_free (dest_entry);
                    g_free (dest_entry_prefix);
                    return FALSE;
                  }

                destination->data.directory.entries = g_slist_prepend (destination->data.directory.entries, dest_entry);
                break;

              default:
                g_object_unref (temp_descriptor);

                src_uri = g_file_get_uri (src_entry->descriptor);
                g_print ("Skipping non-regular file `%s'.\n", src_uri);
                g_free (src_uri);
                break;
            }
        }
    }

  g_free (dest_entry_prefix);
  return TRUE;
}

static gboolean
rudgiosync_synchronize_internal (RudgiosyncDirectoryEntry **destination,
                                 RudgiosyncDirectoryEntry **source,
                                 gboolean size_only,
                                 gboolean checksum_only,
                                 gboolean delete_unwanted,
                                 const gchar *prefix,
                                 GError **error)
{
  GError *ierror = NULL;
  GFile *temp_descriptor;
  gchar *src_uri;
  gchar *dest_uri;

  gboolean already_modified = FALSE;


  if ((*source)->type == RUDGIOSYNC_DIR_ENTRY_OTHER)
    {
      src_uri = g_file_get_uri ((*source)->descriptor);
      g_print ("Skipping non-regular file `%s'.\n", src_uri);
      g_free (src_uri);

      return TRUE;
    }
  else if ((*source)->type == RUDGIOSYNC_DIR_ENTRY_FILE)
    {
      if (!delete_unwanted && (*destination)->type == RUDGIOSYNC_DIR_ENTRY_DIR)
        {
          src_uri = g_file_get_uri ((*source)->descriptor);
          dest_uri = g_file_get_uri ((*destination)->descriptor);

          g_set_error (error, RUDGIOSYNC_ERROR,
                       RUDGIOSYNC_DIR_PROTECTION_ERROR,
                       "Refusing to replace the directory `%s' with "
                       "the file `%s': %s",
                       dest_uri,
                       src_uri,
                       "Could cause major data loss if the request was not "
                       "intentional, use the --delete argument to override "
                       "this behavior");

          g_free (dest_uri);
          g_free (src_uri);

          return FALSE;
        }
      if ((*destination)->type != RUDGIOSYNC_DIR_ENTRY_FILE)
        {
          dest_uri = g_file_get_uri ((*destination)->descriptor);

          rudgiosync_directory_entry_delete (*destination, &ierror);
          *destination = NULL;

          if (ierror != NULL)
            {
              g_propagate_error (error, ierror);

              g_free (dest_uri);
              return FALSE;
            }

          temp_descriptor = g_file_new_for_uri (dest_uri);
          g_free (dest_uri);

          *destination = create_empty_file (temp_descriptor, checksum_only, &ierror);
          g_object_unref (temp_descriptor);
          if (ierror != NULL)
            {
              g_propagate_error (error, ierror);
              return FALSE;
            }
          already_modified = TRUE;
        }
      g_assert (*destination != NULL);
      g_assert ((*destination)->type == RUDGIOSYNC_DIR_ENTRY_FILE);

      sync_file (*destination, *source, size_only, checksum_only, prefix, already_modified, &ierror);
      if (ierror != NULL)
        {
          g_propagate_error (error, ierror);
          return FALSE;
        }
    }
  else if ((*source)->type == RUDGIOSYNC_DIR_ENTRY_DIR)
    {
      if ((*destination)->type != RUDGIOSYNC_DIR_ENTRY_DIR)
        {
          dest_uri = g_file_get_uri ((*destination)->descriptor);

          rudgiosync_directory_entry_delete (*destination, &ierror);
          *destination = NULL;

          if (ierror != NULL)
            {
              g_propagate_error (error, ierror);

              g_free (dest_uri);
              return FALSE;
            }

          temp_descriptor = g_file_new_for_uri (dest_uri);
          g_file_make_directory (temp_descriptor, NULL, &ierror);
          g_object_unref (temp_descriptor);
          if (ierror != NULL)
            {
              g_propagate_error (error, ierror);

              g_free (dest_uri);
              return FALSE;
            }
          *destination = rudgiosync_directory_entry_new (dest_uri, checksum_only, &ierror);
          g_free (dest_uri);
          if (ierror != NULL)
            {
              g_propagate_error (error, ierror);
              return FALSE;
            }
          already_modified = TRUE;
        }
      g_assert (*destination != NULL);
      g_assert ((*destination)->type == RUDGIOSYNC_DIR_ENTRY_DIR);

      if (delete_unwanted)
        {
          delete_non_present_entries_from_dest (*destination, *source, &ierror);
          if (ierror != NULL)
            {
              g_propagate_error (error, ierror);
              return FALSE;
            }
        }

      sync_directory (*destination, *source, size_only, checksum_only, delete_unwanted, prefix, already_modified, &ierror);
      if (ierror != NULL)
        {
          g_propagate_error (error, ierror);
          return FALSE;
        }
    }

  return TRUE;
}

gboolean
rudgiosync_synchronize (RudgiosyncDirectoryEntry **destination,
                        RudgiosyncDirectoryEntry **source,
                        gboolean size_only,
                        gboolean checksum_only,
                        gboolean delete_unwanted,
                        GError **error)
{
  return rudgiosync_synchronize_internal (destination, source,
                                          size_only, checksum_only, delete_unwanted,
                                          NULL,
                                          error);
}
