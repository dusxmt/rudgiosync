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

/* Support for glib-based error reporting. */

#ifndef _RUDGIOSYNC_ERRORS_H_
#define _RUDGIOSYNC_ERRORS_H_

#include "boiler.h"


/* Return a quark for the Rudgiosync error domain. */
GQuark rudgiosync_error_quark (void);
#define RUDGIOSYNC_ERROR rudgiosync_error_quark ()

enum RudgiosyncError
{
  RUDGIOSYNC_INFO_RETRIEVAL_ERROR,
  RUDGIOSYNC_DIR_PROTECTION_ERROR
};

#endif /* _RUDGIOSYNC_ERRORS_H_ */
