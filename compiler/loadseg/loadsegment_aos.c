/*
    Copyright � 1995-2011, The AROS Development Team. All rights reserved.
    $Id$

    Desc:
    Lang: english
*/
#define DEBUG 0
#include <exec/execbase.h>
#include <exec/memory.h>
#include <dos/dosasl.h>
#include <dos/doshunks.h>
#include <dos/dosextens.h>
#include <proto/exec.h>
#include <proto/arossupport.h>
#include <aros/asmcall.h>
#include <aros/debug.h>
#include <aros/macros.h>

#include "loadseg_intern.h"

static int read_block(BPTR file, APTR buffer, ULONG size, SIPTR * funcarray, SIPTR *error, struct Library * lib);

#define GETHUNKPTR(x) ((UBYTE*)(BADDR(hunktab[x]) + sizeof(BPTR)))

/* Seek forward by count ULONGs.
 * Returns 0 on success, 1 on failure.
 */
static int seek_forward(BPTR fd, ULONG count, SIPTR *funcarray, SIPTR *error, struct Library *lib)
{
    int err = 0;
    ULONG tmp;

    /* For AOS compatibility, we can't use DOS/Seek() here,
     * as AOS callers to LoadSegment will not pass
     * in a Seek element of the funcarray, and the read
     * callback of funcarray may be for reading in-memory
     * instead of pointing to DOS/Read.
     *
     * Luckily, reading HUNKs is linear, so we can just
     * read ahead.
     */
    while (count && !(err = read_block(fd, &tmp, sizeof(tmp), funcarray, error, lib)))
    	count--;

    return err;
}

BPTR LoadSegment_AOS(BPTR fh,
                     BPTR table,
                     SIPTR * funcarray,
                     LONG  * stacksize,
                     SIPTR * error,
                     struct Library    * lib)
{
  #define ERROR(a)    { *error=a; goto end; }

  BPTR *hunktab = BADDR(table);
  BPTR firsthunk = BNULL, prevhunk = BNULL;
  ULONG hunktype, count, first, last, curhunk, numhunks;
  LONG t;
  UBYTE name_buf[255];
  register int i;
  BPTR last_p = 0;
  UBYTE *overlaytable = NULL;
  ULONG tmp, req;
#if DEBUG
  static STRPTR segtypes[] = { "CODE", "DATA", "BSS", };
#endif

  if (lib)
    error =&((struct Process *)FindTask(NULL))->pr_Result2;

  curhunk = 0; /* keep GCC quiet */
  /* start point is HUNK_HEADER + 4 */
  while (1)
  {
    if (read_block(fh, &count, sizeof(count), funcarray, error, lib))
      goto end;
    if (count == 0L)
      break;
    count = AROS_BE2LONG(count);
    count *= 4;
    if (read_block(fh, name_buf, count, funcarray, error, lib))
      goto end;
    D(bug("\tlibname: \"%.*s\"\n", count, name_buf));
  }
  if (read_block(fh, &numhunks, sizeof(numhunks), funcarray, error, lib))
    goto end;

  numhunks = AROS_BE2LONG(numhunks);

  D(bug("\tHunk count: %ld\n", numhunks));

  if (!hunktab) {
    hunktab = ilsAllocVec(sizeof(BPTR) * (numhunks + 1 + 1), MEMF_CLEAR);
    if (hunktab == NULL)
      ERROR(ERROR_NO_FREE_STORE);
  }

  if (read_block(fh, &first, sizeof(first), funcarray, error, lib))
    goto end;

  first = AROS_BE2LONG(first);

  D(bug("\tFirst hunk: %ld\n", first));
  curhunk = first;
  if (read_block(fh, &last, sizeof(last), funcarray, error, lib))
    goto end;

  last = AROS_BE2LONG(last);

  D(bug("\tLast hunk: %ld\n", last));
        
  for (i = first; i <= numhunks; i++) {
    UBYTE *hunkptr;
    ULONG hunksize;

    if (i > last) {
      hunktab[i] = BNULL;
      continue;
    }

    if (read_block(fh, &count, sizeof(count), funcarray, error, lib))
      goto end;

    count = AROS_BE2LONG(count);

    tmp = count & 0xFF000000;
    count &= 0xFFFFFF;
    D(bug("\tHunk %d size: 0x%06lx bytes in ", i, count*4));
    req = MEMF_CLEAR;

    switch(tmp)
    {
      case HUNKF_FAST:
      D(bug("FAST"));
      req |= MEMF_FAST;
      break;

      case HUNKF_CHIP:
      D(bug("CHIP"));
      req |= MEMF_CHIP;
      break;

      case HUNKF_ADVISORY:
      D(bug("ADVISORY"));
      if (read_block(fh, &req, sizeof(req), funcarray, error, lib))
        goto end;
      req = AROS_BE2LONG(req);
      break;

      default:
      D(bug("ANY"));
      req |= MEMF_ANY;
      break;
    }

    D(bug(" memory"));
    /*
     * We need space for the code, the length of this hunk and
     * for a pointer to the next hunk.
     * Note also MEMF_31BIT flag appended to memory requirements.
     * This is important on 64-bit machines where AmigaDOS hunk
     * files need to be loaded into low memory (<2GB). This is needed
     * for correct interpretation of pointers in these files.
     * Yes, they can't be executed in such environments, but they still can
     * be read as data files. This allows to use Amiga bitmap fonts on AROS.
     */
    hunksize = count * 4 + sizeof(ULONG) + sizeof(BPTR);
    hunkptr = ilsAllocVec(hunksize, req | MEMF_31BIT);
    if (!hunkptr)
      ERROR(ERROR_NO_FREE_STORE);
    hunktab[i] = MKBADDR(hunkptr);
    D(bug(" @%p\n", hunkptr));
    if (!firsthunk)
      firsthunk = hunktab[i];
      /* Link hunks
         if this is not the first hunk that is loaded, then connect
         it to the previous one (pointer to the field where the
         pointer to the next hunk is located)
       */
    if (prevhunk)
      ((BPTR *)(BADDR(prevhunk)))[0] = hunktab[i];
    prevhunk = hunktab[i];
  }

  while(!read_block(fh, &hunktype, sizeof(hunktype), funcarray, error, lib))
  {
    hunktype = AROS_BE2LONG(hunktype);
    D(bug("Hunk Type: %d\n", hunktype & 0xFFFFFF));

    switch(hunktype & 0xFFFFFF)
    {
      case HUNK_SYMBOL:
        /* The SYMBOL_HUNK looks like this:
             ---------------------
             | n = size of       |  This
             |   symbol in longs |  may
             |-------------------|  be
             | n longwords = name|  repeated
             | of symbol         |  any
             |-------------------|  number
             | value (1 long)    |  of times
             --------------------|
             | 0 = end of HUNK_  |
             |       SYMBOL      |
             --------------------   */

        D(bug("HUNK_SYMBOL (skipping)\n"));
          while(!read_block(fh, &count, sizeof(count), funcarray, error, lib) && count)
          {
            count = AROS_BE2LONG(count) ;

            if (seek_forward(fh, count+1, funcarray, error, lib))
              goto end;
          }
      break;

      case HUNK_UNIT:

        if (read_block(fh, &count, sizeof(count), funcarray, error, lib))
          goto end;

        count = AROS_BE2LONG(count) ;

        count *= 4;
        if (read_block(fh, name_buf, count, funcarray, error, lib))
          goto end;
        D(bug("HUNK_UNIT: \"%.*s\"\n", count, name_buf));
        break;

      case HUNK_CODE:
      case HUNK_DATA:
      case HUNK_BSS:

        if (read_block(fh, &count, sizeof(count), funcarray, error, lib))
          goto end;

          count = AROS_BE2LONG(count);

          D(bug("HUNK_%s(%d): Length: 0x%06lx bytes in ",
          segtypes[(hunktype & 0xFFFFFF)-HUNK_CODE], curhunk, count*4));

        switch(hunktype & 0xFF000000)
        {
          case HUNKF_FAST:
            D(bug("FAST"));
            req = MEMF_FAST;
          break;

          case HUNKF_CHIP:
            D(bug("CHIP"));
            req = MEMF_CHIP;
          break;

          case HUNKF_ADVISORY:
            D(bug("ADVISORY"));
            if (read_block(fh, &req, sizeof(req), funcarray, error, lib))
              goto end;

            req = AROS_BE2LONG(req);

          break;

          default:
            D(bug("ANY"));
            req = MEMF_ANY;
          break;
        }

        D(bug(" memory\n"));
        if ((hunktype & 0xFFFFFF) != HUNK_BSS && count)
	{
          if (read_block(fh, GETHUNKPTR(curhunk), count*4, funcarray, error, lib))
            goto end;

    	}
      break;

      case HUNK_RELOC32:
        D(bug("HUNK_RELOC32:\n"));
        while (1)
        {
          ULONG *addr;
          ULONG offset;

          if (read_block(fh, &count, sizeof(count), funcarray, error, lib))
            goto end;
          if (count == 0L)
            break;

          count = AROS_BE2LONG(count);

          i = count;
          if (read_block(fh, &count, sizeof(count), funcarray, error, lib))
            goto end;

          count = AROS_BE2LONG(count);

          D(bug("\tHunk #%ld:\n", count));
          while (i > 0)
          {
            if (read_block(fh, &offset, sizeof(offset), funcarray, error, lib))
              goto end;

            offset = AROS_BE2LONG(offset);

            //D(bug("\t\t0x%06lx\n", offset));
            addr = (ULONG *)(GETHUNKPTR(curhunk) + offset);

            /* See the above MEMF_31 explanation for why this
             * works on AROS 64-bit.
             */
            *addr = (ULONG)(AROS_BE2LONG(*addr) + (IPTR)GETHUNKPTR(count));

            --i;
          }
        }
      break;

      case HUNK_DREL32: /* For compatibility with V37 */
      case HUNK_RELOC32SHORT:
        {
          ULONG Wordcount = 0;
          ULONG offset;

          while (1)
          {
            ULONG *addr;
    	    UWORD word;
	    
	    Wordcount++;
	    
            if (read_block(fh, &word, sizeof(word), funcarray, error, lib))
              goto end;
            if (word == 0L)
              break;

            word = AROS_BE2LONG(word);

            i = word;
	    Wordcount++;
            if (read_block(fh, &word, sizeof(word), funcarray, error, lib))
              goto end;

            word = AROS_BE2WORD(word);

    	    count = word;
            D(bug("\tHunk #%ld @%p: %ld relocations\n", count, GETHUNKPTR(curhunk), i));
            while (i > 0)
            {
              Wordcount++;
              /* read a 16bit number (2 bytes) */
              if (read_block(fh, &word, sizeof(word), funcarray, error, lib))
                goto end;

              /* offset now contains the byte offset in it`s 16 highest bits.
                 These 16 highest bits have to become the 16 lowest bits so
                 we get the word we need.  */
              offset = AROS_BE2WORD(word);

              D(bug("\t\t0x%06lx += 0x%lx\n", offset, GETHUNKPTR(count)));
              addr = (ULONG *)(GETHUNKPTR(curhunk) + offset);

              /* See the above MEMF_31 explanation for why this
               * works on AROS 64-bit.
               */
              *addr = (ULONG)(AROS_BE2LONG(*addr) + (IPTR)GETHUNKPTR(count));

              --i;
            } /* while (i > 0)*/
          } /* while (1) */

          /* if the amount of words read was odd, then skip the following
           16-bit word   */
          if (0x1 == (Wordcount & 0x1)) {
            UWORD word;
            read_block(fh, &word, sizeof(word), funcarray, error, lib);
          }
        }
      break;

      case HUNK_END:
      {
        D(bug("HUNK_END\n"));
        ++curhunk;
        /* lib == NULL: Called from RDB filesystem loader which does not
         * know filesystem's original size. Exit if last HUNK_END. This can't
         * be done normally because it would break overlayed executables.
         */
        if (!lib && curhunk > last)
            goto done;
      }
      break;

      case HUNK_RELOC16:
        bug("HUNK_RELOC16 not implemented\n");
        ERROR(ERROR_BAD_HUNK);

      case HUNK_RELOC8:
        bug("HUNK_RELOC8 not implemented\n");
        ERROR(ERROR_BAD_HUNK);

      case HUNK_NAME:
        bug("HUNK_NAME not implemented\n");
        ERROR(ERROR_BAD_HUNK);

      case HUNK_EXT:
        bug("HUNK_EXT not implemented\n");
        ERROR(ERROR_BAD_HUNK);

      case HUNK_DEBUG:
        if (read_block(fh, &count, sizeof(count), funcarray, error, lib))
          goto end;

        count = AROS_BE2LONG(count);

        D(bug("HUNK_DEBUG (%x Bytes)\n",count));
        if (seek_forward(fh, count, funcarray, error, lib))
          goto end;
        break;

      case HUNK_OVERLAY:
      {
        D(bug("HUNK_OVERLAY:\n"));
        if (table) /* overlay inside overlay? */
          ERROR(ERROR_BAD_HUNK);
        if (read_block(fh, &count, sizeof(count), funcarray, error, lib))
          goto end;
        count = AROS_BE2LONG(count);
        D(bug("Overlay table size: %d\n", count));

        /* See above for MEMF_31BIT explanation */
        count = count * 4 + sizeof(ULONG) + sizeof(ULONG);
        overlaytable = ilsAllocVec(count, MEMF_CLEAR | MEMF_31BIT);
        if (overlaytable == NULL)
          ERROR(ERROR_NO_FREE_STORE);
        if (read_block(fh, overlaytable, count - sizeof(ULONG), funcarray, error, lib))
            goto end;
        goto done;
      }

      case HUNK_BREAK:
        D(bug("HUNK_BREAK\n"));
        if (!table)
          ERROR(ERROR_BAD_HUNK);
        goto done;

      default:
        bug("Hunk type 0x%06lx not implemented\n", hunktype & 0xFFFFFF);
        ERROR(ERROR_BAD_HUNK);
    } /* switch */
  } /* while */
done:
  if (hunktab)
  {
    ULONG hunksize;
    /* Clear caches */
    for (t = first; t < numhunks && t <= last; t++)
    {
      hunksize = *((ULONG*)BADDR(hunktab[t]) - 1);
      /* We check for SysBase's lib_Version, since some
       * users of this library will be running on AOS 1.3 or lower
       */
      if (hunksize && SysBase->LibNode.lib_Version >= 36)
      {
        CacheClearE(BADDR(hunktab[t]), hunksize, CACRF_ClearI | CACRF_ClearD);
      }
    }

    if (table)
      return firsthunk;

    hunksize = *((ULONG*)BADDR(hunktab[0]) - 1);
    if (last > first && hunksize >= 32 / 4)
    {
      /* NOTE: HUNK_OVERLAY is not required for overlay mode. */
      ULONG *h = (ULONG*)(BADDR(hunktab[first]));

      if (h[2] == 0x0000abcd)
      {
#if __WORDSIZE != 32
        D(bug("overlay not supported!\n"));
        ERROR(ERROR_BAD_HUNK);
#else
        /* overlay executable */
        h[3] = (ULONG)fh;
        h[4] = (ULONG)overlaytable;
        h[5] = (ULONG)MKBADDR(hunktab);
        D(bug("overlay loaded!\n"));
        return (BPTR)(-(LONG)MKBADDR(h));
#endif
      }
    }

    if (overlaytable) {
      ilsFreeVec(overlaytable);
      ERROR(ERROR_BAD_HUNK);
    }

    last_p = firsthunk;

    ilsFreeVec(hunktab);
    hunktab = NULL;
  }

end:
  if (hunktab != NULL)
  {
    for (t = 0 /* first */; t < numhunks /* last */; t++)
      ilsFreeVec(BADDR(hunktab[t]));
    ilsFreeVec(hunktab);
  }
  return last_p;
} /* LoadSegment */


static int read_block(BPTR file, APTR buffer, ULONG size, SIPTR * funcarray, SIPTR *error, struct Library * lib)
{
  LONG subsize;
  UBYTE *buf=(UBYTE *)buffer;

  while(size)
  {
    subsize = ilsRead(file, buf, size);
    if(subsize==0)
    {
      *error = ERROR_BAD_HUNK;
      return 1;
    }

    if(subsize<0)
      return 1;
    buf  +=subsize;
    size -=subsize;
  }
  return 0;
}

