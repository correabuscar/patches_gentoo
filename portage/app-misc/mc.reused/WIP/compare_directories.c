/*
   src/filemanager - tests for comparing directories (ie. F9-c-c)

   Copyright (C) 2011-2019
   Free Software Foundation, Inc.

   Written by:
   Slava Zanko <slavazanko@gmail.com>, 2011, 2013

   This file is part of the Midnight Commander.

   The Midnight Commander is free software: you can redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of the License,
   or (at your option) any later version.

   The Midnight Commander is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define TEST_SUITE_NAME "/src/filemanager"

#include "tests/mctest.h"

#include "src/vfs/local/local.c"

#include "src/filemanager/midnight.c"

//#include "src/filemanager/ext.c"

#include "src/filemanager/cmd.c"

#ifndef MC_CONFIGURE_ARGS
#error "config.h not included? should not happen"
#endif


//src: https://stackoverflow.com/questions/6943862/is-there-a-a-define-for-64-bit-in-gcc/6943917#6943917
#include <limits.h>
#if ( __WORDSIZE == 64 )
#define BUILD_64   1
#endif

#ifdef BUILD_64 //src: https://stackoverflow.com/questions/2467418/portable-way-to-deal-with-64-32-bit-time-t/2467501#2467501
  #define TT_MOD "ll"
#else
  #define TT_MOD ""
#endif

//#define showstate(state, panel) \
//  (state & panel == panel ? "  marked" : "unmarked")
/* --------------------------------------------------------------------------------------------- */
/* mocked functions */


/* --------------------------------------------------------------------------------------------- */
#define ERRORSTRCAP 4096
char * fail_msg=NULL; // when unset, it uses default assertion msg!

//TODO: make it a function with va_args? too lazy. (because ",##__VA_ARGS__" is gcc extension aka non-standard so that it eats up the "," when no args!)
#define setfailmsg(fmt, ...) \
  do { \
    if (NULL == fail_msg) { /* shouldn't be needed here, but just in case it's forgotten in setup() - ok change of plans! */ \
      fail_msg=calloc(1,ERRORSTRCAP); \
    } \
    int size=snprintf(fail_msg, ERRORSTRCAP, fmt, ## __VA_ARGS__ ); \
    if ((size >= ERRORSTRCAP) && (NULL != fail_msg)) { \
      fail_msg[ERRORSTRCAP-1]='\0'; \
    } else if ((size<=0)||(NULL == fail_msg)) { \
      fail_msg="Couldn't create fail msg!"; \
    } \
  } while(0)

static void
setup (void)
{
    str_init_strings (NULL);

    vfs_init ();
    init_localfs ();
    vfs_setup_work_dir ();

    mc_global.mc_run_mode = MC_RUN_FULL;
    left_panel = g_new0 (WPanel, 1);
    left_panel->cwd_vpath = vfs_path_from_str ("/leftdir");
    left_panel->dir.size = DIR_LIST_MIN_SIZE; // 128
    left_panel->dir.list = g_new0 (file_entry_t, left_panel->dir.size);
    left_panel->dir.len = 0;
    //left_panel->type=view_listing; //probably not needed
    //left_panel->active=1;
    //current_panel=left_panel; // probably not needed for anything!

    right_panel = g_new0 (WPanel, 1);
    right_panel->cwd_vpath = vfs_path_from_str ("/rightdir");
    right_panel->dir.size = DIR_LIST_MIN_SIZE;
    right_panel->dir.list = g_new0 (file_entry_t, right_panel->dir.size);
    right_panel->dir.len = 0;
    //right_panel->type=view_listing; //probably not needed
    //fail_msg=malloc(ERRORSTRCAP);
    //setfailmsg("No fail message was set!"); //XXX: don't set this, then it shows the actual assertion that was defined
    use_dash(FALSE); //avoid a segfault in rotate_dash (show=0)
}

static void
teardown (void)
{
  free(fail_msg); fail_msg=NULL;
    vfs_shut ();
    str_uninit_strings ();
    left_panel=NULL;
    right_panel=NULL;
}

/* --------------------------------------------------------------------------------------------- */

/*
Rules: file/symlink names must be the same in both pannels, sizes can differ, mtimes can differ, file type can differ(normal or symlink, or etc)

   file 0b
syml >=1b

 */
void compare(enum CompareMode mode) {
  compare_dir(left_panel, right_panel, mode);
  compare_dir(right_panel, left_panel, mode);
}

//make only one file exit (with name 'fn') in one of the panels('whichpane'=left or right) with 'size' bytes and 'mtime'
#define SPAWNFILE(whichpane, fn, size, mtime) \
  do { \
    whichpane##_panel->dir.len=1; \
    whichpane##_panel->dir.list[whichpane##_panel->dir.len - 1].fname = (char *) fn; \
    whichpane##_panel->dir.list[whichpane##_panel->dir.len - 1].st.st_size = size; \
    whichpane##_panel->dir.list[whichpane##_panel->dir.len - 1].st.st_mtime = mtime; \
  } while (0)

// the only file in one of the panels('whichpane'=left or right) is either marked or unmarked ('yesno'=1 or 0, TRUE or FALSE)
#define ASSERT_MARKED(whichpane, yesno) \
  mctest_assert_int_eq(whichpane##_panel->marked, (int) yesno); //ie. 1=marked, 0=unmarked

//no files are marked(aka selected ie. with yellow color) in either of the panels !
#define ASSERT_NONE_MARKED() \
  do { \
    ASSERT_MARKED(left,0); \
    ASSERT_MARKED(right,0); \
  } while (0)

/* *INDENT-OFF* */
START_TEST (test1)
/* *INDENT-ON* */
{
    /* given */
    SPAWNFILE(left, "somefile2.txt", 0, 1557859190);
    SPAWNFILE(right, "somefile2.txt", 0, 1557859190);

    /* when */
    // 0=_("&Quick"), 1=_("&Size only"), 2=_("&Thorough") from function compare_dirs_cmd() in file src/filemanager/cmd.c
    compare(compare_thourough);

    /* then */
    ASSERT_NONE_MARKED();
}
/* *INDENT-OFF* */
END_TEST
/* *INDENT-ON* */

/* *INDENT-OFF* */
START_TEST (test2)
/* *INDENT-ON* */
{
    /* given */
    SPAWNFILE(left, "somefile2.txt", 0, 1557859190);
    SPAWNFILE(right, "somefile2.txt", 0, 1557859191);

    /* when */
    // 0=_("&Quick"), 1=_("&Size only"), 2=_("&Thorough") from function compare_dirs_cmd() in file src/filemanager/cmd.c
    compare(compare_thourough);

    /* then */
    ASSERT_MARKED(left,0);
    ASSERT_MARKED(right,0);
}
/* *INDENT-OFF* */
END_TEST
/* *INDENT-ON* */

/* *INDENT-OFF* */
START_TEST (test3)
/* *INDENT-ON* */
{
    /* given */
    SPAWNFILE(left, "somefile2.txt", 0, 1557859191);
    SPAWNFILE(right, "somefile2.txt", 0, 1557859190);

    /* when */
    // 0=_("&Quick"), 1=_("&Size only"), 2=_("&Thorough") from function compare_dirs_cmd() in file src/filemanager/cmd.c
    compare(compare_thourough);

    /* then */
    ASSERT_MARKED(left,0);
    ASSERT_MARKED(right,0);
}
/* *INDENT-OFF* */
END_TEST
/* *INDENT-ON* */

struct one_panelfile //ie. one file on any one/single panel, what does this file consist of? metadata-wise
{
  const char *filename;//seen in panel
  const char *symlinkpointsto;//full path(rel/abs) of where the symlink points to, or if not a symlink then NULL! so this is used to know if it's a file or a symlink!
  off_t     st_size; // size of file (or size of symlink, if symlink; ie. not size of the file that the symlink points to!)
  //mode_t    st_mode;        /* File type and mode */
  time_t mtime; // st_mtim.tv_sec - aka the modification time eg. 1557859190 (run: `date -d @1557859190`) - note: can't use st_mtime here (it must be #define-d somewhere!)
  //mark_state selected_which;
};

//typedef enum {
//  NONE_PANEL=0, //no files are selected aka marked, in both panels
//  RITE_PANEL=1, // the file on the right panel is selected (yes, there can be only one file on the right panel, such are these tests composed)
//  LEFT_PANEL=2, // the file on the left panel is selected
//  BOTH_PANEL=3 // the file on both panels is selected
//} file_marked_on; //made to be bit OR-ed
////typedef unsigned int mark_state; // 0=none
//typedef uint8_t mark_state; // 0=none

typedef enum {
  UNINITED=0,
  //---
  MARKED=1,
  UNMARKED=2,
  //---
  LAST_INVALID,
} mark_state; // marked or unmarked

static struct one_panelfile const panel_possibilities[] =
{
  {"somefile.txt", NULL, 0, 1557859190},
  {"somefile.txt", NULL, 0, 1557859191},
  {"somefile.txt", NULL, 1, 1557859190},
};

//#define MAX_CRAZIES 12
#define MAX_CRAZIES (sizeof(panel_possibilities) / sizeof(struct one_panelfile))

static mark_state const matrix[MAX_CRAZIES][MAX_CRAZIES]= { // matrix[left_panel][right_panel] where each panel goes through panel_possibilities ie. in a double for; the value is whether the left panel has its single file(there's only one file per panel always, in these tests) marked aka selected(with yellow color) or not. Since this is a matrix, the case where the panels get swapped is also hit, hence why it's enough to just test for left panel markedness.
  //ie. each row is what's on the left panel, and each column is what's on the right panel; the value(marked/unmarked) referes to the left panel only! was the file on the left panel marked?!
  {UNMARKED, UNMARKED,   MARKED},
  {UNMARKED, UNMARKED,   MARKED},
  {  MARKED, UNMARKED,   MARKED},

  //{UNINITED, UNINITED, UNINITED} //marker to detect when you forgot to init all values ie. when you added more items to panel_possibilities!
};

//static char* const matrix_descriptions[MAX_CRAZIES][MAX_CRAZIES]= {
//  {"same filename,0 bytes size,mtime", "same filename, 0 bytes size, more recent mtime on right panel"},
//  {"same filename,0 bytes size, more recent mtime on left panel", "same filename, 0bytes size, mtime"},
//};

///* *INDENT-OFF* */
//START_TEST (loopytest)
//  /* *INDENT-ON* */
//{
//  // pre-check:
//  ck_assert_int_eq(left_panel->marked, 0);
//  ck_assert_int_eq(right_panel->marked, 0);
//  ck_assert_int_ge(_i, 0);
//  ck_assert_int_le(_i, MAX_CRAZIES);
//  /* given */
//  SPAWNFILE(left, panel_possibilities[_i].filename, panel_possibilities[_i].st_size, panel_possibilities[_i].mtime);
//  SPAWNFILE(right, panel_possibilities[0].filename, panel_possibilities[0].st_size, panel_possibilities[0].mtime);
//  /* when */
//  //compare(compare_thourough);
//  compare_dir(left_panel, right_panel, mode); //compare_dir only marks files in the first(aka left) panel !
//  /* then */
//  //sanity/assumptions check:
//  ck_assert_int_ge(left_panel->marked, 0);
//  ck_assert_int_le(left_panel->marked, 1);
//  ck_assert_int_ge(right_panel->marked, 0);
//  ck_assert_int_le(right_panel->marked, 1);
////  ck_assert_int_eq(MAX_CRAZIES, 2);//temp hardcoded, remove me TODO:
//  if (1 == left_panel->marked) {
//    ck_assert_int_eq(2, (left_panel->marked << 1) );
//  }
//  //
//  const mark_state expected=matrix[_i][0];
//  _ck_assert_int(expected, >=, 0);
//  ck_assert_int_le(expected, BOTH_PANEL);
//  const mark_state current=( (left_panel->marked << 1) | (right_panel->marked) );
//  setfailmsg("FAILed for _i=%d as follows:\n\
//left  panel:%s size:%jd mtime:%08" TT_MOD "d\n\
//right panel:%s size:%jd mtime:%08" TT_MOD "d\n\
//result  : left: %s, right: %s\n\
//expected: left: %s, right: %s\n\
//------\n", _i,
//      panel_possibilities[_i].filename, panel_possibilities[_i].st_size, panel_possibilities[_i].mtime,
//      panel_possibilities[0].filename, panel_possibilities[0].st_size, panel_possibilities[0].mtime,
//      showstate(current, LEFT_PANEL),
//      showstate(current, RITE_PANEL),
//      showstate(expected, LEFT_PANEL),
//      showstate(expected, RITE_PANEL)
//      );
//  //ck_assert_msg(current == expected, matrix_descriptions[_i][0]);
//  ck_assert_msg(current == expected, fail_msg);
//}
///* *INDENT-OFF* */
//END_TEST
///* *INDENT-ON* */

/* *INDENT-OFF* */
START_TEST (allinone)
  /* *INDENT-ON* */
{
  for (unsigned int left_index=0; left_index < MAX_CRAZIES; left_index++) {
    for (unsigned int right_index=0; right_index < MAX_CRAZIES; right_index++ ) {

      left_panel->dir.len = 0;
      right_panel->dir.len = 0;
      left_panel->marked=0;
      right_panel->marked=0;

      setfailmsg("You did not init matrix[][] for all possible values!");
      ck_assert_msg( matrix[left_index][right_index] != UNINITED, fail_msg);

  // pre-check:
      ck_assert(NULL != left_panel);
      ck_assert(NULL != right_panel);
  ck_assert_int_eq(left_panel->marked, 0);
  ck_assert_int_eq(right_panel->marked, 0);
//  ck_assert_int_ge(_i, 0);
//  ck_assert_int_le(_i, MAX_CRAZIES);
  /* given */
  SPAWNFILE(left,
      panel_possibilities[left_index].filename,
      panel_possibilities[left_index].st_size,
      panel_possibilities[left_index].mtime);
  SPAWNFILE(right,
      panel_possibilities[right_index].filename,
      panel_possibilities[right_index].st_size,
      panel_possibilities[right_index].mtime);
  /* when */
  compare_dir(left_panel, right_panel, compare_thourough); //compare_dir only marks files in the first(aka left) panel !
  /* then */
  //sanity/assumptions check:
  ck_assert_int_ge(left_panel->marked, 0); // zero...
  ck_assert_int_le(left_panel->marked, 1); // or one files got marked on left panel, always.
  ck_assert_int_eq(right_panel->marked, 0); // none was marked on the right panel, always!
//  ck_assert_int_eq(MAX_CRAZIES, 2);//temp hardcoded, remove me TODO:
  //
  const mark_state expected=matrix[left_index][right_index];
  _ck_assert_int(expected, >, UNINITED);
  _ck_assert_int(expected, <, LAST_INVALID);
  const mark_state current = ( left_panel->marked == 1 ? MARKED : UNMARKED );
  setfailmsg("FAILed for matrix[%u,%u] as follows:\n\
left  panel:%s size:%jd mtime:%08" TT_MOD "d\n\
right panel:%s size:%jd mtime:%08" TT_MOD "d\n\
left panel had the file  : %s\n\
but it was expected to be: %s\n\
------\n", left_index, right_index,
      panel_possibilities[left_index].filename, panel_possibilities[left_index].st_size, panel_possibilities[left_index].mtime,
      panel_possibilities[right_index].filename, panel_possibilities[right_index].st_size, panel_possibilities[right_index].mtime,
      (current == UNMARKED ? "unmarked":"marked"),
      (expected == MARKED ? "marked":"unmarked")
//      showstate(current, LEFT_PANEL),
//      showstate(current, RITE_PANEL),
//      showstate(expected, LEFT_PANEL),
//      showstate(expected, RITE_PANEL)
      );
  //ck_assert_msg(current == expected, matrix_descriptions[_i][0]);
  ck_assert_msg(current == expected, fail_msg);

    }
  }
}
/* *INDENT-OFF* */
END_TEST
/* *INDENT-ON* */

/* --------------------------------------------------------------------------------------------- */

int
main (void)
{
    int number_failed;

    Suite *s = suite_create (TEST_SUITE_NAME);
    TCase *tc_core = tcase_create ("Core");
    SRunner *sr;

    tcase_add_checked_fixture (tc_core, setup, teardown);

    /* Add new tests here: *************** */
    tcase_add_test (tc_core, test1);
    tcase_add_test (tc_core, test2);
    tcase_add_test (tc_core, test3);
    //tcase_add_loop_test(tc_core, loopytest, 0, MAX_CRAZIES);
    tcase_add_test (tc_core, allinone);
    /* *********************************** */

    suite_add_tcase (s, tc_core);
    sr = srunner_create (s);
    srunner_set_log (sr, "compare_directories.log");
    srunner_run_all (sr, CK_ENV);
    number_failed = srunner_ntests_failed (sr);
    srunner_free (sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* --------------------------------------------------------------------------------------------- */
