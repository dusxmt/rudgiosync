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
#include "errors.h"


static RudgiosyncDirectoryEntry *
rudgiosync_directory_entry_new_internal (const gchar *uri, GFile *descriptor, GFileInfo *info, gboolean checksum_wanted, GError **error)
{
  RudgiosyncDirectoryEntry  *retval;
  GFileEnumerator           *enumerator;
  const gchar               *string_attr;

  RudgiosyncDirectoryEntry  *child_entry;
  GFileInfo                 *child_info;
  GFile                     *child_descriptor;
  gchar                     *child_uri;

  GError *ierror = NULL;


  retval = g_slice_new0 (RudgiosyncDirectoryEntry);
  retval->descriptor = g_object_ref (descriptor);

  retval->modified_time = g_file_info_get_attribute_uint64 (info, G_FILE_ATTRIBUTE_TIME_MODIFIED);
  switch (g_file_info_get_attribute_uint32 (info, G_FILE_ATTRIBUTE_STANDARD_TYPE))
    {
      case G_FILE_TYPE_REGULAR:
        retval->type = RUDGIOSYNC_DIR_ENTRY_FILE;
        break;

      case G_FILE_TYPE_DIRECTORY:
        retval->type = RUDGIOSYNC_DIR_ENTRY_DIR;
        break;

      default:
        retval->type = RUDGIOSYNC_DIR_ENTRY_OTHER;
        break;
    }

  string_attr = g_file_info_get_attribute_byte_string (info, G_FILE_ATTRIBUTE_STANDARD_NAME);
  if (string_attr == NULL)
    {
      g_set_error (error, RUDGIOSYNC_ERROR,
                   RUDGIOSYNC_INFO_RETRIEVAL_ERROR,
                   "Failed to retrieve information about the file `%s': %s",
                   uri,
                   "Filename information missing in GFileInfo");

      rudgiosync_directory_entry_free (retval);
      return NULL;
    }
  retval->name = g_strdup (string_attr);

  string_attr = g_file_info_get_attribute_string (info, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME);
  if (string_attr == NULL)
    {
      g_set_error (error, RUDGIOSYNC_ERROR,
                   RUDGIOSYNC_INFO_RETRIEVAL_ERROR,
                   "Failed to retrieve information about the file `%s': %s",
                   uri,
                   "Displayable filename information missing in GFileInfo");

      rudgiosync_directory_entry_free (retval);
      return NULL;
    }
  retval->display_name = g_strdup (string_attr);

  switch (retval->type)
    {
      case RUDGIOSYNC_DIR_ENTRY_FILE:
        retval->data.file.size = g_file_info_get_attribute_uint64 (info, G_FILE_ATTRIBUTE_STANDARD_SIZE);
        if (checksum_wanted)
          {
            rudgiosync_checksum_for_gfile (retval->descriptor,
                                           &(retval->data.file.checksum),
                                           &ierror);
            if (ierror != NULL)
              {
                g_propagate_prefixed_error (error, ierror, "Failed to produce a checksum for the file `%s': ", uri);

                rudgiosync_directory_entry_free (retval);
                return NULL;
              }
          }
        break;

      case RUDGIOSYNC_DIR_ENTRY_DIR:
        enumerator = g_file_enumerate_children (retval->descriptor,
                                                G_FILE_ATTRIBUTE_STANDARD_TYPE ","
                                                G_FILE_ATTRIBUTE_STANDARD_NAME ","
                                                G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME ","
                                                G_FILE_ATTRIBUTE_STANDARD_SIZE ","
                                                G_FILE_ATTRIBUTE_TIME_MODIFIED,
                                                G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                                NULL,
                                                &ierror);
        if (ierror != NULL)
          {
            g_propagate_prefixed_error (error, ierror, "Failed to retrieve information about the children of the directory `%s': ", uri);

            rudgiosync_directory_entry_free (retval);
            return NULL;
          }
        while (TRUE)
          {
            child_info = g_file_enumerator_next_file (enumerator, NULL, &ierror);
            if (ierror != NULL)
              {
                g_propagate_prefixed_error (error, ierror, "Failed to retrieve information about a child of the directory `%s': ", uri);

                g_object_unref (enumerator);
                rudgiosync_directory_entry_free (retval);

                return NULL;
              }
            if (child_info == NULL)
              {
                break;
              }
            string_attr = g_file_info_get_attribute_byte_string (child_info, G_FILE_ATTRIBUTE_STANDARD_NAME);
            if (string_attr == NULL)
              {
                g_set_error (error, RUDGIOSYNC_ERROR,
                             RUDGIOSYNC_INFO_RETRIEVAL_ERROR,
                             "Failed to retrieve information about a child of the directory `%s': %s",
                             uri,
                             "Filename information missing in GFileInfo retrieved from GFileEnumerator");

                g_object_unref (child_info);
                g_object_unref (enumerator);
                rudgiosync_directory_entry_free (retval);

                return NULL;
              }
            child_descriptor = g_file_get_child (retval->descriptor, string_attr);
            child_uri = g_file_get_uri (child_descriptor);
            child_entry = rudgiosync_directory_entry_new_internal (child_uri, child_descriptor, child_info, checksum_wanted, &ierror);
            g_free (child_uri);
            g_object_unref (child_descriptor);
            g_object_unref (child_info);

            if (ierror != NULL)
              {
                g_propagate_prefixed_error (error, ierror, "Failed to retrieve information about a child of the directory `%s': ", uri);

                g_object_unref (enumerator);
                rudgiosync_directory_entry_free (retval);

                return NULL;
              }

            retval->data.directory.entries = g_slist_prepend (retval->data.directory.entries, child_entry);
          }

        g_object_unref (enumerator);
        break;
    }

  return retval;
}

RudgiosyncDirectoryEntry *
rudgiosync_directory_entry_new (GFile *descriptor, gboolean checksum_wanted, GError **error)
{
  RudgiosyncDirectoryEntry *retval;

  gchar      *uri;
  GFileInfo  *info;
  GError     *ierror = NULL;

  uri = g_file_get_uri (descriptor);
  info = g_file_query_info (descriptor,
                            G_FILE_ATTRIBUTE_STANDARD_TYPE ","
                            G_FILE_ATTRIBUTE_STANDARD_NAME ","
                            G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME ","
                            G_FILE_ATTRIBUTE_STANDARD_SIZE ","
                            G_FILE_ATTRIBUTE_TIME_MODIFIED,
                            G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                            NULL,
                            &ierror);
  if (ierror != NULL)
    {
      g_propagate_prefixed_error (error, ierror, "Failed to retrieve information about the file `%s': ", uri);

      g_free (uri);
      return NULL;
    }

  retval = rudgiosync_directory_entry_new_internal (uri, descriptor, info, checksum_wanted, error);

  g_object_unref (info);
  g_free (uri);

  return retval;
}


void
rudgiosync_directory_entry_free (gpointer to_free_in)
{
  RudgiosyncDirectoryEntry *to_free = (RudgiosyncDirectoryEntry *)to_free_in;

  if (to_free == NULL)
    return;

  if (to_free->descriptor != NULL)
    {
      g_object_unref (to_free->descriptor);
    }

  g_free (to_free->name);
  g_free (to_free->display_name);

  switch (to_free->type)
    {
      case RUDGIOSYNC_DIR_ENTRY_DIR:
        g_slist_free_full (to_free->data.directory.entries, rudgiosync_directory_entry_free);
        break;
    }

  g_slice_free (RudgiosyncDirectoryEntry, to_free);
}
