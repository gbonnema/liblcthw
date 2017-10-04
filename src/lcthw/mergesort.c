#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <lcthw/dbg.h>

#include <lcthw/mergesort.h>

typedef struct
{

} part_list_t;
typedef struct
{
} part_info_t;

/* prototypes */
int merge_move_mem(size_t member_size, void *to, void *from);
int merge(void *array, size_t nmem, size_t member_size, char *scratch,
          compare_t compare, size_t left, size_t llen, size_t right, size_t rlen);
int __merge(void *array, size_t nmem, size_t member_size, char *scratch,
            compare_t compare, size_t left, size_t llen, size_t right, size_t rlen,
            void (*print)(void * const array, const size_t nmem, const void *temp));

/**
 * Moves memory of a specified size from one location to another and
 * initializes the from location.
 *
 * @param member_size size_t the amount of memory being moved
 * @param to void * pointer to the destination
 * @param from void * pointer to the origin
 * @return zero if all went well, -1 if an error occurred.
 *
 * Remark: this function has no way of checking the validity of the
 * memory addresses. This is the callers responsibility.
 */
int merge_move_mem(size_t member_size, void *to, void *from)
{
    check(member_size != 0, "Member size can not be zero.");
    check(to != NULL, "Cannot copy memory to NULL.");
    check(from != NULL, "Cannot copy memory from NULL.");

    char *ret = memcpy(to, from, member_size);
    check(ret != NULL, "Memory copy in hiring phase failed");

    ret = memset(from, '\0', member_size);
    check(ret != NULL, "Memory init of vacancy position failed");

    return 0;
error:
    return -1;
}

/**
 * Sorts an array `array[1..nmem]` of member_size `len` into ascending order according
 * to a compare function.
 *
 * We divide the array into nmem / n equal parts of n members where n doubles each run.
 *
 * The iteration works bottom up from partitions of 1 member doubling its length
 * each run. In the last run only one whole partition of n members remains and
 * a tail where nmem/2 < n <= nmem. If n == nmem then no more sorting will be done
 * as the tail has length zero.
 *
 * @param array void * The pointer to the array.
 * @param nmem size_t the number of members
 * @param member_size size_t the size of one member of the array
 * @param compare compare_t function pointer to compare function
 * @return zero if all is well, -1 if something went wrong.
 *
 */
int mergesort(void *array, size_t nmem, size_t member_size, compare_t compare)
{
    return __mergesort(array, nmem, member_size, compare, NULL);
}

int __mergesort(void *array, size_t nmem, size_t member_size, compare_t compare,
                void (*print)(void * const array, const size_t nmem, const void *temp)
               )
{
    (void)(print);

    char *scratch = NULL;

    check(array != NULL, "Input array cannot be NULL");
    check(compare != NULL, "Compare function cannot be NULL");

    size_t nparts;			/** The number of full partitions. */
    size_t npaired_parts;	/** The number of parts to be compared of equal size */
    _Bool has_single_part;		/** The last odd part if present */
    size_t part_len;		/** The length of a partition */
    size_t tail_len;		/** The length of the last partition */

    /** Create a scratch area of equal size to the array */
    scratch = malloc(nmem * member_size);
    errno = ENOMEM;
    check(scratch != NULL, "Error allocating scratch");

    /*
     * divide the array into nmem / n equal parts of n members where n doubles each run.
     */
    for(size_t n=1; n<nmem; n<<=1)
    {
        nparts = nmem / n;				/* Integer division: truncates */
        npaired_parts = nparts / 2;
        has_single_part = nparts % 2;
        part_len = n;					/* Yes, this is duplicate */
        tail_len = nmem % n;

#ifndef NDEBUG
        /* useful for debugging sort */
        printf("n = %lu, nparts = %zd, npaired_parts = %zd, "
               "has_single_part = %s, part_len = %zd, tail_len = %zd.\n",
               n, nparts, npaired_parts, has_single_part ? "true" : "false", part_len, tail_len);
#endif
        /* merge the sets of partitions of equal size nmem / n */
        for (size_t i = 0; i < npaired_parts; i++)
        {
            size_t left_offset = 2*i*part_len;
            size_t right_offset = left_offset + part_len;
#ifdef NDEBUG
            int r = merge(array, nmem, member_size, scratch, compare,
                          left_offset, part_len, right_offset, part_len);
            check(r == 0,
                  "merge failed. (array, nmem, member_size) = (%p, %zd, %zd)\n"
                  "(scratch                 ) = (%p)\n"
                  "(left_offset, part_len   ) = (%zd, %zd)\n"
                  "(right_offset            ) = (%zd)\n",
                  array, nmem, member_size, scratch, left_offset, part_len, right_offset);
#else
            if(print)
            {
                printf("-------------------\n");
                printf("array before merge (left_offset, right_offset) = (%zd, %zd):\n",
                       left_offset, right_offset);
                printf("       real array:\t");
                print(array, nmem, NULL);
                printf("    scratch array:\t");
                print(scratch, nmem, NULL);
            }
            int r = __merge(array, nmem, member_size, scratch, compare,
                            left_offset, part_len, right_offset, part_len, print);
            if(print)
            {
                printf("array after merge (left_offset, right_offset) = (%zd, %zd):\n",
                       left_offset, right_offset);
                printf("       real array:\t");
                print(array, nmem, NULL);
                printf("    scratch array:\t");
                print(scratch, nmem, NULL);
            }
            check(r == 0,
                  "__merge failed. (array, nmem, member_size) = (%p, %zd, %zd)\n"
                  "(scratch                 ) = (%p)\n"
                  "(left_offset, part_len   ) = (%zd, %zd)\n"
                  "(right_offset            ) = (%zd)\n",
                  array, nmem, member_size, scratch, left_offset, part_len, right_offset);
#endif
        }

        if (has_single_part == 1)
        {
            /* Merge last 2 partitions: if tail is empty nothing happens. */
            size_t start_single_part = 2 * npaired_parts * part_len;
            size_t start_tail = start_single_part + part_len;
#ifdef NDEBUG
            merge(array, nmem, member_size, scratch, compare,
                  start_single_part, part_len, start_tail, tail_len);
#else
            __merge(array, nmem, member_size, scratch, compare,
                    start_single_part, part_len, start_tail, tail_len, print);
#endif
        }
    }

    /* Free the allocated array */
    if (scratch) free(scratch);
    return 0;
error:
    /* Free the allocated array */
    if (scratch) free(scratch);
    return -1;
}
/**
 * merges two sorted segments of an array keeping the elements in order and
 * preserving the order between equal keys.
 *
 * @param array void * The pointer to the array.
 * @param nmem size_t the number of members
 * @param member_size size_t the size of one member of the array
 * @param compare compare_t function pointer to compare function
 * @param left size_t the offset of the left partition
 * @param llen size_t the length of the left partition
 * @param right size_t the offset of the right partition
 * @param rlen size_t the length of the right partition
 * if defined NDEBUG:
 *    @param print void (*)(void * const array, const size_t nmem, const void *temp)
 *					The print function provided by caller: is allowed to be NULL.
 * @return zero if all is well, -1 if something went wrong.
 */
int merge(void *array, size_t nmem, size_t member_size, char *scratch,
          compare_t compare, size_t left, size_t llen, size_t right, size_t rlen)
{
    return __merge(array, nmem, member_size, scratch, compare, left, llen, right, rlen, NULL);
}

int __merge(void *array, size_t nmem, size_t member_size, char *scratch,
            compare_t compare,
            size_t left, size_t llen, size_t right, size_t rlen,
            void (*print)(void * const array, const size_t nmem, const void *temp)
           )
{
    (void)(print);

    /* Check the input */
    errno = EINVAL;
    check(array != NULL, "Cannot merge a NULL array.");
    errno = EINVAL;
    check(member_size != 0, "member_size cannot be zero.");
    errno = EINVAL;
    check(scratch != NULL, "Scratch area should not be null.");
    errno = EINVAL;
    check(compare != NULL, "Compare should not be NULL");
    errno = EINVAL;
    check(left+llen <= nmem,
          "Left partition should not extend beyond array: (left, llen, nmem) = (%lu, %lu, %lu).",
          left, llen, nmem);
    errno = EINVAL;
    check(right+rlen <= nmem,
          "Right partition should not extend beyond array: (right, rlen, nmem) = (%lu, %lu, %lu).",
          right, rlen, nmem);
    errno = EINVAL;
    check(left+llen == right, "Partitions should be consecutive");

    /* Guard for empty arrays and arrays with 1 element */
    if (nmem < 2) return 0;

    /* Guard for empty partitions */
    if (llen == 0) return 0;
    if (rlen == 0) return 0;

    /* Now we start the merge */

    /* calculate the total length of both partitions together */
    size_t tot_len = rlen+llen;
    /* Copy the subsequent partition members to the scratch array in one go */
    void *scratch_addr = scratch + left * member_size;
    void *array_addr = array + left * member_size;
    memcpy(scratch_addr, array_addr, tot_len * member_size);
    memset(array_addr, '\0', tot_len * member_size);

    /* Merge both array parts from the allocated array to the original array */
    size_t i = left;
    size_t j = left + llen;
    size_t merge_pt = left;
    while (merge_pt - left < tot_len)
    {
        check(i - left + j - left - llen  == merge_pt - left,
              "Merge point somehow degraded: (i, j, merge_pt) = (%zd, %zd, %zd)."
              "\n(left, llen) = (%zd, %zd)",
              i, j, merge_pt, left, llen);
        if (print)
        {
            printf("(left, llen, rlen) = (%zd, %zd, %zd)", left, llen, rlen);
            printf("(i, j) = (%zd, %zd) ", i, j);
            printf("(merge_pt) = (%zd)\n", merge_pt);
            printf("array  : ");
            print(array, nmem, NULL);
            printf("scratch: ");
            print(scratch, nmem, NULL);
        }
        if (i - left < llen && j - left < tot_len)
        {
            /* compare and move smallest to destination */
            void *left_addr = scratch + i * member_size;
            void *right_addr = scratch + j * member_size;
            if (compare(left_addr, right_addr) <= 0)
            {
                int r = merge_move_mem(member_size, array+merge_pt*member_size,
                                       scratch+i*member_size);
                check(r == 0, "merge_move_mem of memory chunk left partition failed.");
                i++;
                merge_pt++;
            }
            else
            {
                int r = merge_move_mem(member_size, array+merge_pt*member_size,
                                       scratch+j*member_size);
                check(r == 0, "merge_move_mem of memory chunk right partition failed.");
                j++;
                merge_pt++;
            }
        }
        else if (i - left < llen)
        {
            /* move left partition to destination */
            int r = merge_move_mem(member_size, array+merge_pt*member_size,
                                   scratch+i*member_size);
            check(r == 0, "merge_move_mem of memory chunk left partition failed.");
            i++;
            merge_pt++;
        }
        else if (j - left < tot_len)
        {
            /* move right partition to destination */
            int r = merge_move_mem(member_size, array+merge_pt*member_size,
                                   scratch+j*member_size);
            check(r == 0, "merge_move_mem of memory chunk right partition failed.");
            j++;
            merge_pt++;
        }

    }

    return 0;
error:
    return -1;
}
