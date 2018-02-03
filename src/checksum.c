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
#include "checksum.h"

#include <string.h>

#ifdef RUDGIOSYNC_CHECKSUM_ENABLED
void
rudgiosync_hash_init (RudgiosyncHashContext *context)
{
  sha256_init (context);
}

void
rudgiosync_hash_data (RudgiosyncHashContext *context,
                      gsize length, const gchar *data)
{
  sha256_update (context, length, (const uint8_t *)data);
}

void
rudgiosync_hash_finish (RudgiosyncHashContext *context,
                        RudgiosyncChecksum *checksum)
{
  sha256_digest (context, SHA256_DIGEST_SIZE, checksum->sha256_digest);
}

void
rudgiosync_checksum_display (RudgiosyncChecksum *checksum)
{
  gsize iter;

  g_print ("sha256: ");
  for (iter = 0; iter < SHA256_DIGEST_SIZE; iter++)
    {
      g_print ("%02x", (checksum->sha256_digest)[iter]);
    }
}


gboolean
rudgiosync_checksums_differ (RudgiosyncChecksum *checksum_a,
                             RudgiosyncChecksum *checksum_b)
{
  return (memcmp (checksum_a->sha256_digest, checksum_b->sha256_digest,
                  SHA256_DIGEST_SIZE)) != 0;
}


#define HASH_BUF_SIZE ((gsize)(2 * 1024 * 1024)) /* 2 MiB */

gboolean
rudgiosync_checksum_for_gfile (GFile *descriptor,
                               RudgiosyncChecksum *checksum,
                               GError **error)
{
  RudgiosyncHashContext  hash_context;
  GFileInputStream      *input_stream;

  gssize  read_count;
  gchar  *hash_buf;

  GError *ierror = NULL;


  input_stream = g_file_read (descriptor, NULL, &ierror);
  if (ierror != NULL)
    {
      g_propagate_error (error, ierror);
      return FALSE;
    }
  hash_buf = g_new (gchar, HASH_BUF_SIZE);
  rudgiosync_hash_init (&hash_context);
  while (TRUE)
    {
      read_count = g_input_stream_read (G_INPUT_STREAM (input_stream),
                                        hash_buf, HASH_BUF_SIZE,
                                        NULL, &ierror);
      if (ierror != NULL)
        {
          g_propagate_error (error, ierror);

          g_free (hash_buf);
          g_object_unref (input_stream);
          return FALSE;
        }

      if (read_count == 0)
        break;

      rudgiosync_hash_data (&hash_context, (gsize)read_count, hash_buf);
    }
  g_free (hash_buf);
  g_object_unref (input_stream);

  rudgiosync_hash_finish (&hash_context, checksum);
  return TRUE;
}


#else /* !RUDGIOSYNC_CHECKSUM_ENABLED */
void
rudgiosync_hash_init (RudgiosyncHashContext *context)
{
  g_error ("Checksum support disabled at compile time.");
}

void
rudgiosync_hash_data (RudgiosyncHashContext *context,
                      gsize length, const gchar *data)
{
  g_error ("Checksum support disabled at compile time.");
}

void
rudgiosync_hash_finish (RudgiosyncHashContext *context,
                        RudgiosyncChecksum *checksum)
{
  g_error ("Checksum support disabled at compile time.");
}

void
rudgiosync_checksum_display (RudgiosyncChecksum *checksum)
{
  g_error ("Checksum support disabled at compile time.");
}

gboolean
rudgiosync_checksum_for_gfile (GFile *descriptor,
                               RudgiosyncChecksum *checksum,
                               GError **error)
{
  g_error ("Checksum support disabled at compile time.");
  return FALSE;
}

gboolean
rudgiosync_checksums_differ (RudgiosyncChecksum *checksum_a,
                             RudgiosyncChecksum *checksum_b)
{
  g_error ("Checksum support disabled at compile time.");
  return FALSE;
}

#endif /* !RUDGIOSYNC_CHECKSUM_ENABLED */
